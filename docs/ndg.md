# ndgrammar rules
This file is incomplete. You may be better off reading the ndg spec itself, in [`grammars/ndg.ndg`](../grammars/ndg.ndg).

Various methods exist to capture characters:
- Ranges: `[A-Za-z0-9_]`
- Strings: `'foo'` or `"bar"`
- Wildcards: `.`

Quantity may be specified as a suffix:
- `*`: Zero or more occurances
- `+`: One or more occurances
- `?`: Zero or one occurances

Groups can be formed by encasing multiple capture rules in parenthesis.

Delimiters can be applied to both quantity suffixes and groups with an additional proceeding token, respectively delimited by `:` and `%`.

## Lexer rules
Lexer rules are defined with an uppercase starting character.
Rules may be prefixed with `~` to tell the lexer to ignore them.

## Parser rules
Parser rules are defined with a lowercase starting character.

Fields may be denoted with the `:` operator. Resulting nodes will contain a field with the provided name and data.

Flags may be denoted with the `@` operator. Resulting nodes will contain a boolean with the flag name, set true if the match registers.