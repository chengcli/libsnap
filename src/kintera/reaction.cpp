// C/C++
#include <sstream>

// snap
#include <snap/utils/vectorize.hpp>

#include "reaction.hpp"

namespace snap {

bool starts_with(const std::string& s, const std::string& prefix) {
  return s.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string& s, const std::string& suffix) {
  return s.size() >= suffix.size() &&
         s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

double fp_value_check(const std::string& s) {
  std::istringstream ss(s);
  double value;
  ss >> value;
  if (ss.fail()) {
    throw std::runtime_error("Error parsing floating point value");
  }
  return value;
}

Reaction::Reaction(const std::string& equation) {
  // Parse the reaction equation to determine participating species and
  // stoichiometric coefficients
  auto tokens = Vectorize<std::string>(equation.c_str(), " ");
  tokens.push_back("+");  // makes parsing last species not a special case

  //! index returned by functions to indicate "no position"
  const auto npos = static_cast<size_t>(-1);

  size_t last_used = npos;  // index of last-used token
  bool is_reactants = true;
  for (size_t i = 1; i < tokens.size(); i++) {
    if (tokens[i] == "+" || starts_with(tokens[i], "(+") ||
        tokens[i] == "<=>" || tokens[i] == "=" || tokens[i] == "=>") {
      std::string species = tokens[i - 1];

      double stoich = 1.0;
      bool mass_action = true;
      if (last_used != npos && tokens[last_used] == "(+" &&
          ends_with(species, ")")) {
        // Falloff third body with space, such as "(+ M)"
        mass_action = false;
        species = "(+" + species;
      } else if (last_used == i - 1 && starts_with(species, "(+") &&
                 ends_with(species, ")")) {
        // Falloff 3rd body written without space, such as "(+M)"
        mass_action = false;
      } else if (last_used == i - 2) {
        // Species with no stoich. coefficient
      } else if (last_used == i - 3) {
        // Stoich. coefficient and species
        stoich = fp_value_check(tokens[i - 2]);
      } else {
        throw std::runtime_error("Error parsing reaction string");
      }

      if (is_reactants) {
        reactants()[species] += stoich;
      } else {
        products()[species] += stoich;
      }

      last_used = i;
    }

    // Tokens after this point are part of the products string
    if (tokens[i] == "<=>" || tokens[i] == "=") {
      reversible(true);
      is_reactants = false;
    } else if (tokens[i] == "=>") {
      reversible(false);
      is_reactants = false;
    }
  }
}

std::string Reaction::equation() const {
  if (reversible()) {
    return to_string(reactants()) + " <=> " + to_string(products());
  } else {
    return to_string(reactants()) + " => " + to_string(products());
  }
}

std::string to_string(Composition const& p) {
  std::ostringstream result;
  for (auto iter = p.begin(); iter != p.end(); ++iter) {
    if (iter != p.begin()) {
      result << " + ";
    }
    if (iter->second != 1.0) {
      result << iter->second << " ";
    }
    result << iter->first;
  }
  return result.str();
}

}  // namespace snap
