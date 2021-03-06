THE SOFTWARE IS PROVIDED "AS IS" AND BRIAN SMITH AND THE AUTHORS DISCLAIM
ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL BRIAN SMITH OR THE AUTHORS
BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.



*ring*
======

*ring* is a crypto library in Rust based on BoringSSL's crypto primitive
implementations.

Particular attention is being paid to making it easy to build and integrate
*ring* into applications and higher-level frameworks, and to ensuring that
*ring* works optimally on small devices, and eventually microcontrollers, to
support Internet of Things (IoT) applications.

The name *ring* comes from the fact that *ring* started as a subset of
BoringSSL, and *"ring"* is a substring of "Bo*ring*SSL". Most of the (C and
assembly language) code in *ring* comes from BoringSSL, and BoringSSL is
derived from OpenSSL. *ring* merges changes from BoringSSL regularly. Also,
several changes that were developed for *ring* have already been merged into
BoringSSL.



Documentation
-------------

See the documentation at
https://briansmith.org/rustdoc/ring/.

See [BUILDING.md](BUILDING.md#building-the-rust-library) for instructions on
how to build it. These instructions are especially important on Windows, as
there are build prerequisites that need to be installed.



Contributing
------------

Patches Welcome! Suggestions:

* More code elimination, especially dead code.
* Replacing more C code with Rust code.
* Implementation of [SRP-6a](http://srp.stanford.edu/) in Rust, based on the
  |rust::digest| API and the C/asm optimized modular exponentiation.
* Optimizing the PBKDF2-HMAC implementation based on the ideas from
  [fastpbkdf2](https://github.com/ctz/fastpbkdf2).
* Better IDE support for Windows (e.g. running the tests within the IDE) and
  Mac OS X (e.g. Xcode project files).
* Support for more platforms in the continuous integration (e.g. Android, iOS,
  ARM microcontrollers).
* Static analysis and fuzzing in the continuous integration.



License
-------

See [LICENSE](LICENSE).

The *ring* project happily accepts pull requests without you needing to sign
any formal license agreement. The portions of pull requests that modify
existing files must be licensed under the same terms as the files being
modified. New files in pull requests, including in particular all Rust code,
must be licensed under the ISC-style license. Please state that you agree to
license your contributions in the commit messages of commits in pull requests,
e.g. by putting “I agree to license my contributions to each file under the
terms given at the top of each file I changed.” at the end of each commit
message.



Online Automated Testing
------------------------

Travis CI is used for Linux and Mac OS X. Appveyor is used for Windows. The
tests are run in debug and release configurations, for the current release of
each Rust channel (Stable, Beta, Nightly), for each configuration listed
in the table below.

<table>
<tr><th>OS</th><th>Arch.</th><th>Compilers</th><th>Status</th>
<tr><td rowspan=2>Linux</td>
    <td>x86, x64</td>
    <td>GCC 4.6, GCC 5, Clang 3.8</td>
    <td rowspan=3><a title="Build Status" href=https://travis-ci.org/briansmith/ring><img src=https://travis-ci.org/briansmith/ring.svg?branch=master></a>
</tr>
<tr><td>32-bit ARM, AAarch64</td>
    <td>Linaro GCC 5.1-2015.08 (build only, no tests are run)</td>
</tr>
<tr><td>Mac OS X</td>
    <td>x64</td>
    <td>Apple Clang 7.0.2 (clang-700.1.81)</td>
</tr>
<tr><td>Windows</td>
    <td>x86, x64</td>
    <td>MSVC 2013 Update 5 (12.0), MSVC 2015 Update 1 (14.0)</td>
    <td><a title="Build Status" href=https://ci.appveyor.com/project/briansmith/ring/branch/master><img src=https://ci.appveyor.com/api/projects/status/3wq9p54r9iym05rm/branch/master?svg=true></a>
</tr>
</table>



Bug Reporting
-------------

Please report bugs either as pull requests or as issues in [the issue
tracker](https://github.com/briansmith/ring/issues). *ring* has a
**full disclosure** vulnerability policy. **Please do NOT attempt to report
any security vulnerability in this code privately to anybody.**
