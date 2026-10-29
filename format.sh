#!/bin/bash
find src include -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -exec clang-format-22 -i --style=file {} +
