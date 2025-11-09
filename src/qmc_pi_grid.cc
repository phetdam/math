/**
 * @file qmc_pi_grid.cc
 * @author Derek Huang
 * @brief C++ program writing the QMC pi estimation point grid data
 * @copyright MIT License
 */

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <exception>
#include <string>
#include <string_view>

namespace {

// program name and help text
const auto progname = std::filesystem::path{__FILE__}.stem().string();
const std::string program_usage{
  "Usage: " + progname + " [-h] -n NU [-f FMT] [-o OUTPUT]\n"
  "\n"
  "Write the rectangle rule quasi Monte Carlo pi estimation grid points.\n"
  "\n"
  "The points are written as single-precision floats to the default precision.\n"
  "If no output file is provided values are written to standard output."
  // TODO: add option description
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
 * Command-line argument structure.
 *
 * @param print_usage `true` to print usage
 * @param nu Number of points per axis
 * @param fmt Output format
 * @param out Output file path
 */
struct cli_options {
  bool print_usage = false;
  unsigned nu = 0u;
  output_format fmt = output_format::auto_;
  std::filesystem::path out;
};

// limit on nu
constexpr auto nu_max = std::numeric_limits<decltype(cli_options::nu)>::max();

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
 * Parse action to call when help option is specified.
 *
 * This simply sets `opts.print_usage` to `true`.
 *
 * @param opts Command-line options to fill
 * @param args View of `main()` args
 * @returns `true`
 */
bool parse_help(cli_options& opts, argv_view& /*args*/)
{
  opts.print_usage = true;
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
      "not one of csv, tsv, auto" << std::endl;
    return false;
  }
  return true;
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
    auto action = [&opts, &args]() -> decltype(&parse_args)
    {
      // -h, --help
      if (*args == "-h" || *args == "--help")
        return parse_help;
      // -n
      else if (*args == "-n")
        return parse_nu;
      // -f, --format
      else if (*args == "-f" || *args == "--format")
        return parse_fmt;
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
    if (opts.print_usage)
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
    // otherwise, look at file extension
    else if (ext == ".csv")
      opts.fmt = output_format::csv;
    else if (ext == ".tsv")
      opts.fmt = output_format::tsv;
    // unknown extension
    else {
      std::cerr << "Error: Unknown file format extension " <<
        (ext.empty() ? ext : ext.substr(1u)) << " not one of csv, tsv" <<
        std::endl;
      return false;
    }
  }
  // done
  return true;
}

/**
 * Write float rectangle rule grid points to the output stream.
 *
 * The header labels written are simply `x` and `y`.
 *
 * @param out Output stream to write to
 * @param nu Number of points per axis
 * @param delim Delimiter character
 */
void write_points(std::ostream& out, unsigned nu, char delim = ',')
{
  // write header
  out << 'x' << delim << 'y' << '\n';
  // write values
  for (auto i = 0u; i < nu; i++)
    for (auto j = 0u; j < nu; j++)
      out << ((i + 0.5f) / nu) << delim << ((j + 0.5f) / nu) << '\n';
  // flush to finish write
  out << std::flush;
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
  if (opts.print_usage) {
    std::cout << program_usage << std::endl;
    return EXIT_SUCCESS;
  }
  // set delimiter
  char delim = (opts.fmt == output_format::csv) ? ',' : '\t';
  // if path is empty, write to stdout
  if (opts.out.empty())
    write_points(std::cout, opts.nu, delim);
  // otherwise write to file
  else {
    std::ofstream out{opts.out};
    write_points(out, opts.nu, delim);
  }
  return EXIT_SUCCESS;
}
