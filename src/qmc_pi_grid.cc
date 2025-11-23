/**
 * @file qmc_pi_grid.cc
 * @author Derek Huang
 * @brief C++ program writing the QMC pi estimation point grid data
 * @copyright MIT License
 */

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <string_view>

#include "pdmath/warnings.h"

namespace {

// program name, program name padding as spaces, and help text
const auto progname = std::filesystem::path{__FILE__}.stem().string();
const std::string progname_ws(progname.size(), ' ');
const std::string program_usage{
  "Usage: " + progname + " [-h] -n NU [-m (mt32|rect)] [-f (auto|csv|tsv)]\n"
  "       " + progname_ws + " [-f(mt32|rect)-[OPTION] [VALUE...] ...]\n"
  "       " + progname_ws + " [-o OUTPUT]\n"
  "\n"
  "Write the rectangle rule quasi Monte Carlo pi estimation grid points.\n"
  "\n"
  "The points are written as single-precision floats to the default precision.\n"
  "If no output file is provided values are written to standard output.\n"
  "\n"
  "To facilitate comparison of the rectangle rule points with other typical\n"
  "sampling methods the -m, --method sampling option can be used. The\n"
  "corresponding -f[METHOD]-[OPTION] options can be used to pass method-\n"
  "specific options as required by a particular sampling method.\n"
  "\n"
  "Options:\n"
  "  -h, --help        Print this usage\n"
  "  -n NU             Number of points per dimension. The total number of\n"
  "                    sampled points in [0, 1] x [0, 1] will be NU * NU.\n"
  "\n"
  "  -m (mt32|rect), --method (mt32|rect)\n"
  "                    Point sampling method. \"mt32\" selects the 32-bit\n"
  "                    Mersenne Twister PRNG as implemented in std::mt19937\n"
  "                    while \"rect\" uses the rectangle rule. If not specified\n"
  "                    explicitly the default is \"rect\".\n"
  "\n"
  "  -f (auto|csv|tsv), --format (auto|csv|tsv)\n"
  "                    Output file format. \"csv\" forces comma-separated\n"
  "                    values, \"tsv\" forces tab-separated values, while\n"
  "                    \"auto\" will select based on file extension if an\n"
  "                    output file is specified but will choose tab-separated\n"
  "                    if writing to standard output. \"auto\" is default.\n"
  "\n"
  "  -f(mt32|rect)-[OPTION] [VALUE...]\n"
  "                    Additional options specific to a particular sampling\n"
  "                    method. For example, -fmt32-seed can be used to give a\n"
  "                    starting seed value to std::mt19937.\n"
  "\n"
  "  -o OUTPUT, --output OUTPUT\n"
  "                    Output file to write to instead of standard output"
};

/**
 * Output format enum.
 */
enum output_format {
  auto_,  // determine output format automatically
  csv,    // comma-separated values
  tsv,    // tab-separated values (better for writing to stdout)
};

/**
 * Point sampling method enum.
 */
enum sample_method {
  rect,  // rectangle rule
  mt32   // 32-bit Mersenne Twister
};

/**
 * `main()` argument iterator.
 *
 * This simplifies managing `argc, `argv`, and the position of the current arg.
 * It can be seen as an implementation of the C++20 `input_iterator`.
 */
class argv_view {
public:
  /**
   * Ctor.
   *
   * @param argc `main()` argument count
   * @param argv `main()` argument vector
   * @param i Current argument index
   */
  argv_view(int argc, char** argv, int i = 0) noexcept
    : argc_{argc}, argv_{argv}, i_{i}
  {}

  /**
   * Advance the index of the argument to be examined via pre-increment.
   */
  auto& operator++() noexcept
  {
    next();
    return *this;
  }

  /**
   * Advance the index of the argument to be examined via post-increment.
   */
  auto operator++(int) noexcept
  {
    auto cur = *this;
    next();
    return cur;
  }

  /**
   * Indicate if any arguments have not yet been examined yet.
   *
   * In other words, this function returns `true` as long as the index of the
   * current argument is still within the bounds of the argument count.
   *
   * This can also be used as an "arguments remaining" check.
   */
  operator bool() const noexcept
  {
    return i_ < argc_;
  }

   /**
   * Return a view to the current argument being examined.
   *
   * Behavior is undefined if `operator bool()` returns `false`.
   */
  std::string_view operator*() const noexcept
  {
    return argv_[i_];
  }

private:
  int argc_;
  char** argv_;
  int i_;

  /**
   * Advance the index of the argument to be examined.
   */
  void next() noexcept
  {
    i_++;
  }
};

/**
 * Sampling method `argv_view` visitor.
 *
 * This provides a polymorphic interface for handling sampling method specific
 * command-line options, encapsulating the corresponding state, and
 * implementing the `write()` method required to write the grid points.
 */
class point_sampler {
public:
  /**
   * Dtor.
   */
  virtual ~point_sampler() = default;

  /**
   * Return the associated sampling method enumeration value.
   */
  virtual sample_method type() const = 0;

  /**
   * Check if the current command-line argument is a sampler-specific option.
   *
   * This is used in `parse_args()` to determine if any sampler-specific
   * parsing needs to be done before `parse()` is called.
   *
   * @returns `true` if current argument is sampler-specific, `false` otherwise
   */
  virtual bool can_parse(argv_view& /*args*/) const
  {
    return false;
  }

  /**
   * Parse a sampler-specific option and its values.
   *
   * `can_parse()` *must* already return `true` on args and at least one
   * `-f[METHOD]-[OPTION]` argument and value(s) should be consumed. This
   * function can populate internal state.
   *
   * @returns `true` on success, `false` on error
   */
  virtual bool parse(argv_view& /*args*/)
  {
    std::cerr << "Error: Point sampler does not implement parse()" << std::endl;
    return false;
  }

  /**
   * Write the points sampled from `[0, 1] x [0, 1]` to the output stream.
   *
   * This non-virtual function calls `write_points()` after writing the header
   * labels of `x` and `y` to the given output stream.
   *
   * @note This function is non-const as some RNGs may need to update state.
   *
   * @param out Output stream to write to
   * @param nu Number of points per axis
   * @param delim Delimiter character
   */
  void write(std::ostream& out, unsigned nu, char delim)
  {
    // write header
    out << 'x' << delim << 'y' << '\n';
    // write values + ensure flush
    write_points(out, nu, delim);
    out << std::flush;
  }

private:
  /**
   * Write points sampled from `[0, 1] x [0, 1]` to the output stream.
   *
   * This must be implemented by derived types and has few restrictions.
   *
   * @param out Output stream to write to
   * @param nu Number of points per axis
   * @param delim Delimiter character
   */
  virtual void write_points(std::ostream& out, unsigned nu, char delim) = 0;
};

/**
 * Point sampler implementation for the rectangle rule.
 */
class rect_sampler : public point_sampler {
public:
  /**
   * Return the corresponding sampling method enum value.
   */
  sample_method type() const /* noexcept */ override
  {
    return sample_method::rect;
  }

private:
  /**
   * Write float rectangle rule grid points to the output stream.
   *
   * @param out Output stream to write to
   * @param nu Number of points per axis
   * @param delim Delimiter character
   */
  void write_points(std::ostream& out, unsigned nu, char delim) override
  {
    // write values
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
    for (auto i = 0u; i < nu; i++)
      for (auto j = 0u; j < nu; j++)
        out << ((i + 0.5f) / nu) << delim << ((j + 0.5f) / nu) << '\n';
PDMATH_MSVC_WARNINGS_POP()
  }
};

/**
 * Point sampler implementation for the 32-bit Mersenne Twister.
 */
class mt32_sampler : public point_sampler {
public:
  /**
   * Return the corresponding sampling method enum value.
   */
  sample_method type() const /* noexcept */ override
  {
    return sample_method::mt32;
  }

  /**
   * Check if the current command-line argument is a sampler-specific option.
   *
   * The only option that is accepted is `-fmt32-seed`.
   *
   * @param args Command-line arguments to parse
   * @returns `true` if current argument is sampler-specific, `false` otherwise
   */
  bool can_parse(argv_view& args) const override
  {
    return *args == "-fmt32-seed";
  }

  /**
   * Parse a sampler-specific option and its values.
   *
   * This consumes the `-fmt32-seed` option's argument from args if possible.
   *
   * @param args Command-line arguments to parse
   * @returns `true` on success, `false` on error
   */
  bool parse(argv_view& args) override
  {
    // option name
    constexpr auto opt = "-fmt32-seed";
    // advance and check if arg is available
    if (!++args) {
      std::cerr << "Error: Missing required argument for " << opt << std::endl;
      return false;
    }
    // if available, try to convert to unsigned
    try {
      // seed max limit
      constexpr auto seed_max = std::numeric_limits<std::uint_fast32_t>::max();
      auto seed = std::stoul(std::string{*args});
      // too large
      if (seed >= seed_max) {
        std::cerr << "Error: " << opt << " given value " << seed <<
          " that exceeds allowed maximum " << seed_max << std::endl;
        return false;
      }
      // ok, copy-assign RNG
      rng_ = std::mt19937{seed};
    }
    // handle exceptions
    catch (const std::exception& exc) {
      std::cerr << "Error: " << opt << " value " << *args <<
        " conversion failed: " << exc.what() << std::endl;
      return false;
    }
    // success
    return true;
  }

private:
  std::uniform_real_distribution<float> dist_;
  std::mt19937 rng_;

  /**
   * Write float uniformly-sampled grid points to the output stream.
   *
   * @param out Output stream to write to
   * @param nu Number of points per axis
   * @param delim Delimiter character
   */
  void write_points(std::ostream& out, unsigned nu, char delim) override
  {
    for (auto i = 0u; i < nu; i++)
      for (auto j = 0u; j < nu; j++)
        out << dist_(rng_) << delim << dist_(rng_) << '\n';
  }
};

/**
 * Command-line argument structure.
 *
 * @param help `true` to print usage
 * @param nu Number of points per axis
 * @param fmt Output format
 * @param smp Polymorphic point sampler
 * @param out Output file path
 */
struct cli_options {
  bool help = false;
  unsigned nu = 0u;
  output_format fmt = output_format::auto_;
  std::unique_ptr<point_sampler> smp = std::make_unique<rect_sampler>();
  std::filesystem::path out;
};

// limit on nu
constexpr auto nu_max = std::numeric_limits<decltype(cli_options::nu)>::max();

/**
 * Parse action to call when help option is specified.
 *
 * This simply sets `opts.help` to `true`.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true`
 */
bool parse_help(cli_options& opts, argv_view& /*args*/)
{
  opts.help = true;
  return true;
}

/**
 * Parse value of `nu` for the command-line options.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true` on success, `false` on error
 */
bool parse_nu(cli_options& opts, argv_view& args)
{
  // advance and check if arg is available
  if (!++args) {
    std::cerr << "Error: Missing required argument for -n" << std::endl;
    return false;
  }
  // if available, try to convert to unsigned
  try {
    auto nu = std::stoul(std::string{*args});
    // too large
    if (nu >= nu_max) {
      std::cerr << "Error: -n given value " << nu <<
        " that exceeds allowed maximum " << nu_max << std::endl;
      return false;
    }
    // ok, assign to opts.nu
    opts.nu = static_cast<decltype(opts.nu)>(nu);
  }
  // handle exceptions
  catch (const std::exception& exc) {
    std::cerr << "Error: -n argument conversion failed: " << exc.what() <<
      std::endl;
    return false;
  }
  // success
  return true;
}

/**
 * Parse the sampling method string from the command-line options.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true` on success, `false` on error
 */
bool parse_method(cli_options& opts, argv_view& args)
{
  // option name
  constexpr auto opt = "-m, --method";
  // advance and check if arg is available
  if (!++args) {
    std::cerr << "Error: Missing required argument for " << opt << std::endl;
    return false;
  }
  // if available, update sampler as appropriate
  if (*args == "rect") {
    opts.smp = std::make_unique<rect_sampler>();
    return true;
  }
  if (*args == "mt32") {
    opts.smp = std::make_unique<mt32_sampler>();
    return true;
  }
  // unknown, so just error
  std::cerr << "Error: Invalid sampler type " << *args << " provided for " <<
    opt << std::endl;
  return false;
}

/**
 * Parse format value for the command-line options.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true` on success, `false` on error
 */
bool parse_fmt(cli_options& opts, argv_view& args)
{
  // advance and check if next arg is available
  if (!++args) {
    std::cerr << "Error: Missing required argument for -f, --format" <<
      std::endl;
    return false;
  }
  // if available, match to enum
  if (*args == "csv")
    opts.fmt = output_format::csv;
  else if(*args == "tsv")
    opts.fmt = output_format::tsv;
  else if (*args == "auto")
    opts.fmt = output_format::auto_;
  // unknown
  else {
    std::cerr << "Error: Unknown -f, --format value " << *args <<
      " not one of csv, tsv, auto" << std::endl;
    return false;
  }
  return true;
}

/**
 * Parse a sampling method specific option from the command-line options.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true` on success, `false` on error
 */
bool parse_fmethod_option(cli_options& opts, argv_view& args)
{
  return opts.smp->parse(args);
}

/**
 * Parse output file path for the command-line options.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true` on success, `false` on error
 */
bool parse_output(cli_options& opts, argv_view& args)
{
  // advance and check if next arg is available
  if (!++args) {
    std::cerr << "Error: Missing required argument for -o, --output" <<
      std::endl;
    return false;
  }
  // make path object + check that parent path exists
  auto out = std::filesystem::absolute(*args);
  if (!std::filesystem::exists(out.parent_path())) {
    std::cerr << "Error: Specified output path " << out.string() <<
      " parent directory does not exist" << std::endl;
    return false;
  }
  // ok, so assign
  opts.out = std::move(out);
  return true;
}

/**
 * Parse incoming command-line options.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true` on success, `false` on error
 */
bool parse_args(cli_options& opts, argv_view& args)
{
  // consume all arguments
  while (args) {
    // assign parse function pointer
    auto action = [&args, &opts]() -> decltype(&parse_args)
    {
      // -h, --help
      if (*args == "-h" || *args == "--help")
        return parse_help;
      // -n
      else if (*args == "-n")
        return parse_nu;
      // -m, --method
      else if (*args == "-m" || *args == "--method")
        return parse_method;
      // -f, --format
      else if (*args == "-f" || *args == "--format")
        return parse_fmt;
      // -f[METHOD]-[OPTION]
      else if (opts.smp->can_parse(args))
        return parse_fmethod_option;
      // -o, --output
      else if (*args == "-o" || *args == "--output")
        return parse_output;
      // otherwise, nullptr
      else
        return nullptr;
    }();
    // unknown argument
    if (!action) {
      std::cerr << "Error: Unknown option " << *args << std::endl;
      return false;
    }
    // parse
    if (!action(opts, args))
      return false;
    // special case: break early if help option seen
    if (opts.help)
      return true;
    // advance
    args++;
  }
  // nu is required
  if (!opts.nu) {
    std::cerr << "Error: Missing required -n option" << std::endl;
    return false;
  }
  // ensure that format and output file are consistent
  if (opts.fmt == output_format::auto_) {
    // file extension
    auto ext = opts.out.extension().string();
    // if file is empty, use TSV
    if (opts.out.empty())
      opts.fmt = output_format::tsv;
    // otherwise, look at file extension. if empty, can't use auto
    else if (ext.empty()) {
      std::cerr << "Error: Cannot deduce output format from file without a " <<
        "file extension" << std::endl;
      return false;
    }
    else if (ext == ".csv")
      opts.fmt = output_format::csv;
    else if (ext == ".tsv")
      opts.fmt = output_format::tsv;
    // unknown extension
    // note: already checked ext.empty() case so substr() will not errir
    else {
      std::cerr << "Error: Unknown file format extension " << ext.substr(1u) <<
        " not one of csv, tsv" << std::endl;
      return false;
    }
  }
  // done
  return true;
}

}  // namespace

int main(int argc, char** argv)
{
  // parse arguments w/ argv view (skipping argv[0])
  cli_options opts{};
  argv_view args{argc, argv, 1};
  if (!parse_args(opts, args))
    return EXIT_FAILURE;
  // print usage if requested
  if (opts.help) {
    std::cout << program_usage << std::endl;
    return EXIT_SUCCESS;
  }
  // set delimiter
  char delim = (opts.fmt == output_format::csv) ? ',' : '\t';
  // if path is empty, write to stdout
  if (opts.out.empty())
    opts.smp->write(std::cout, opts.nu, delim);
  // otherwise write to file
  else {
    std::ofstream out{opts.out};
    opts.smp->write(out, opts.nu, delim);
  }
  return EXIT_SUCCESS;
}
