###################################################################################
#
#    Envy Game Engine
#    https://github.com/PatrickTorgerson/Envy
#
#    Copyright (c) 2021 Patrick Torgerson
#
#    Permission is hereby granted, free of charge, to any person obtaining a copy
#    of this software and associated documentation files (the "Software"), to deal
#    in the Software without restriction, including without limitation the rights
#    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#    copies of the Software, and to permit persons to whom the Software is
#    furnished to do so, subject to the following conditions:
#
#    The above copyright notice and this permission notice shall be included in all
#    copies or substantial portions of the Software.
#
#    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#
###################################################################################


def generate_mainpage():

    """
    Modifies './README.md' for use as the documentation's main page
    makes changes to take advantage of doxygen features and improve apperance
    as HTML
    """

    with open("../README.md") as readme_md:
        with open("./mainpage.md", 'w') as mainpage_md:
            for line in readme_md:

                if line == "# Envy\n":
                    # Header ID, tells doxygen to use as main page
                    mainpage_md.write("# Envy {#mainpage}\n")
                elif line == "[![Lines of Code](https://tokei.rs/b1/github.com/PatrickTorgerson/Envy?category=code)](https://github.com/PatrickTorgerson/Envy)\n":
                    # Add a badge to ling to the github repo
                    mainpage_md.write("[![GitHub](https://img.shields.io/badge/GitHub-Envy-blue)](https://github.com/PatrickTorgerson/Envy)\n")
                    mainpage_md.write(line)
                elif line.startswith('` '):
                    # Block quote code line cuz it looks better in the html
                    mainpage_md.write('> ')
                    mainpage_md.write(line)
                else:
                    mainpage_md.write(line)

    return


if __name__ == '__main__':
    generate_mainpage()