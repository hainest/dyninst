/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "pathName.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <string>

// Replace unix `~` in a path with $HOME
static std::string expand_tilde(std::string path_name) {
  if (path_name.empty() || path_name[0] != '~') {
    return path_name;
  }

  char const *home_dir = std::getenv("HOME");
  if (!home_dir) {
    return path_name;
  }

  // A tilde by itself (e.g. ~/x or ~)
  if (path_name[1] == '/' || path_name[1] == '\0') {
    return path_name.replace(0, 1, home_dir);
  }

  // A tilde followed by a username.
  // The username starts at path_name[1] and ends at the first '/' or '\0'.
  auto const idx_of_slash = path_name.find('/');

  if (idx_of_slash == std::string::npos) {
    return home_dir;
  }

  return home_dir + path_name.substr(idx_of_slash);
}

std::string extract_pathname_tail(const std::string &path) {
  boost::filesystem::path p(path);
  return p.filename().string();
}

std::string resolve_file_path(std::string path) {
  namespace ba = boost::algorithm;
  namespace bf = boost::filesystem;

  // Remove all leading and trailing spaces in-place.
  ba::trim(path);

#ifndef os_windows
  // On Linux-like OSes, collapse doubled directory separators
  // similar to POSIX `realpath`. On Windows, '//' is a (possibly)
  // meaningful separator, so don't change it.
  ba::replace_all(path, "//", "/");
#endif

  // If it has a tilde, expand tilde pathname
  // This is a no-op on Windows
  if (path.find('~') != std::string::npos) {
    path = expand_tilde(path);
  }

  // Convert to a boost::filesystem::path
  // This makes a copy of `path`.
  auto boost_path = bf::path(path);

  // bf::canonical (see below) requires that the path exists.
  if (!bf::exists(boost_path)) {
    return {};
  }

  /* Make the path canonical
   *
   * This converts the path to an absolute path (relative to the
   * current working directory) that has no symbolic links, '.',
   * or '..' elements and strips trailing directory separator.
   *
   * NOTE: makes a copy of the path.
   */
  boost::system::error_code ec;
  auto canonical_path = bf::canonical(boost_path, ec);
  if (ec != boost::system::errc::success) {
    return {};
  }

  /* This is a bit strange, but is most optimal as it is the only
   * member string-conversion function that does not inhibit moving
   * the return value out.
   */
  return canonical_path.string<std::string>();
}
