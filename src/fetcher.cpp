// fetcher.cpp: the HTTPS requests to Binance, paging, and updating the CSV cache.
// Syllabus: Unit 2 constructor and destructor, Unit 4 rethrowing an exception.

#include "fetcher.hpp"

#include <curl/curl.h>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <thread>

#include "errors.hpp"

namespace quant {

static const int baseUrlCount = 3;
static const std::string baseUrls[baseUrlCount] = {"https://api.binance.com",
                                                   "https://data-api.binance.vision",
                                                   "https://api1.binance.com"};
static const int pageSize = 1000;
static const long long msPerDay = 86400000LL;

// libcurl calls this for each chunk of the reply. Its signature must match what libcurl expects.
static size_t storeBody(char* chunk, size_t size, size_t count, void* userData) {
    std::string* body = static_cast<std::string*>(userData);
    body->append(chunk, size * count);
    return size * count;
}

// Outside the MSYS2 shell libcurl cannot find its certificate bundle, so we point it at one.
// Returns "" when there is none, and the caller then uses the Windows certificate store.
static std::string findCaBundle() {
    const char* fromEnvironment = std::getenv("CURL_CA_BUNDLE");
    if (fromEnvironment != nullptr && std::ifstream(fromEnvironment).good()) return fromEnvironment;
    std::string msys2Bundle = "C:/msys64/ucrt64/etc/ssl/certs/ca-bundle.crt";
    if (std::ifstream(msys2Bundle).good()) return msys2Bundle;
    return "";
}

DataFetcher::DataFetcher() {
    handle = curl_easy_init();
    if (handle == nullptr) throw NetworkError("libcurl could not start");

    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, storeBody);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "quant-advisor-course-project");

    // Certificate checks stay on, always. Only where the certificates come from can change.
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
    std::string caBundle = findCaBundle();
    if (caBundle.empty()) {
        curl_easy_setopt(handle, CURLOPT_SSL_OPTIONS, static_cast<long>(CURLSSLOPT_NATIVE_CA));
    } else {
        curl_easy_setopt(handle, CURLOPT_CAINFO, caBundle.c_str());
    }
}

DataFetcher::~DataFetcher() noexcept {
    curl_easy_cleanup(handle);
}

std::string DataFetcher::httpGet(const std::string& url) {
    std::string body;
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &body);

    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK) {
        throw NetworkError(std::string(curl_easy_strerror(result)) + " (" + url + ")");
    }

    long status = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status);
    if (status == 451) throw NetworkError("HTTP 451: Binance blocks this region (" + url + ")", status);
    if (status != 200) throw NetworkError("HTTP " + std::to_string(status) + " from " + url, status);
    return body;
}

std::string DataFetcher::getFromAnyBase(const std::string& pathAndQuery) {
    for (int i = 0; i < baseUrlCount; i++) {
        try {
            return httpGet(baseUrls[i] + pathAndQuery);
        } catch (const NetworkError& error) {
            bool rateLimited = error.httpStatus() == 429 || error.httpStatus() == 418;
            bool lastAddress = i == baseUrlCount - 1;
            // Unit 4: rethrowing, the caller must hear about it when we have to stop trying.
            if (rateLimited || lastAddress) throw;
            std::cerr << error.what() << ", trying the next address\n";
        }
    }
    return "";  // never reached: the last address either returns or rethrows
}

std::vector<Candle> DataFetcher::fetchRange(const std::string& symbol, long long startMs) {
    std::vector<Candle> all;
    long long nextStart = startMs;
    long long now = currentTimeMs();

    while (true) {
        std::string query = "/api/v3/klines?symbol=" + symbol + "&interval=1h&startTime=" +
                            std::to_string(nextStart) + "&limit=" + std::to_string(pageSize);
        std::vector<Candle> page = parseKlines(getFromAnyBase(query), now);
        all.insert(all.end(), page.begin(), page.end());

        int pageCount = page.size();
        if (pageCount < pageSize) break;
        nextStart = page.back().openTime + 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));  // be polite to the public API
    }
    validateCandles(all);  // each page was checked, this also checks the joins between pages
    return all;
}

std::vector<Candle> DataFetcher::fetchLatest(const std::string& symbol) {
    std::string query = "/api/v3/klines?symbol=" + symbol + "&interval=1h&limit=" + std::to_string(pageSize);
    return parseKlines(getFromAnyBase(query), currentTimeMs());
}

void DataFetcher::ensureHistory(const DataStore& store, const std::string& symbol, int days) {
    long long now = currentTimeMs();
    std::vector<Candle> cached;
    if (store.hasFile(symbol)) cached = store.load(symbol);

    long long first = -1;
    if (!cached.empty()) first = cached.front().openTime;

    if (needsFullRefetch(first, now, days)) {
        std::vector<Candle> candles = fetchRange(symbol, now - days * msPerDay);
        if (candles.empty()) throw DataError("Binance returned no candles for " + symbol);
        store.save(symbol, candles);
    } else {
        store.append(symbol, fetchRange(symbol, cached.back().openTime + 1));
    }
}

}  // namespace quant
