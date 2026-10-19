# ndl
Implementing a systems language from half a spec I made while half-asleep around 02:00 am.

You may have questions.
- **Why?** Why not? It's an excuse for me to get more familiar with lower-level systems programming, and I can't stop thinking about the spec.
- **What does it stand for?** That's a mystery. I forgot by the time I woke up the next morning. I wrote `ndl` on the spec sheet, and that's staying.

Currently, no part of ndl is implemented. The first phase of this project is a separate language:

## ndgrammar
Apparently writing one language from scratch isn't enough. ndg is a grammar language; its sole purpose is to construct parse trees. I took inspiration from ANTLR and OMeta, but went in a different direction than either two, with my own odd syntax ideas.

Look at the spec in [`grammars/ndg.ndg`](grammars/ndg.ndg) (or the very WIP [documentation](docs/ndg.md)).

Currently, all the binary does is construct a parse tree from the provided grammar file, then serialize and print it to stdout. Be warned, it has no error handling yet. It will quit without telling you why.

The next step is to write a compiler that takes the constructed tree and generates code to parse other files by the rules it defines. The intent is to bootstrap the language with said compiler.

## Building
Build by running `make`. The output binary will be `bin/ndg`.

```sh
# Build:
make
# Usage:
bin/ndg <grammar_file>
```

The spec for ndg is located under [`grammars/ndg.ndg`](grammars/ndg.ndg). You can test the binary with that file.