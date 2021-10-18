# Functions
## `itoa(int)`
Converts an integer to a string.

Example: `itoa(-34)` returns `"-34"`

## `atoi(str)`
Converts a string to an integer.

Example: `atoi("34")` returns `34`

## `kindof(val)`
Returns one of `"boolean"`, `"integer"`, `"string"`, or `"array"`.

Example: `kindof("12")` returns `"string"`

## `length(str/ary)`
Returns the length of the string or array.

Example: `length("foo")` returns `3`
Example: `length([true, "foo", 9, [1]])` returns `4`

## `insert(ary, idx, ele)`
Modifies the `ary` in place by inserting an element into the array at the specified index. If `idx` is equal to `length(ary)`, add to the end of the array.

Example: given `a=[true, "yup"]`, running `insert(a,2,"foo")` will cause `a` to be `[true, "yup", "foo"]`

## `delete(ary, idx)`
Deletes the element at the given index of the array, returning it.

Example: given `a=[true, "yup"]`, running `delete(a, 1)` should return `"yup"` and cause `a` to be `[true]`.

## `print(str)`
Prints `str` to stdout.

## `prompt()`
Gets a line from stdin.

## `exit(int)`
Exits with the given status code

## `random()`
Returns a random nonnegative number.