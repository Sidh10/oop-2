// demo.hpp: the concepts demo, with small toy classes that show one syllabus topic each.
// Each function prints short labelled lines, so the menu and the tests can both use it.

#ifndef DEMO_HPP
#define DEMO_HPP

#include <iostream>

namespace quant {

void unit2Objects(std::ostream& out);
void unit2Pointers(std::ostream& out);
void unit3Inheritance(std::ostream& out);
void unit4Exceptions(std::ostream& out);
void unit4TemplatesExceptions(std::ostream& out);
void unit5Files(std::ostream& out);

inline void runAllDemos(std::ostream& out) {
    unit2Pointers(out);
    unit3Inheritance(out);
    unit4TemplatesExceptions(out);
    unit5Files(out);
}

}  // namespace quant

#endif
