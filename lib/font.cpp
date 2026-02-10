#include "draw/font.hpp"

#include "draw/sans16.hpp"
#include "draw/sans32.hpp"

namespace draw {

std::array<font const*, 2> all_fonts{&sans16, &sans32};

}  // end namespace draw
