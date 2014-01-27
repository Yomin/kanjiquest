# kanjiquest

Pops open a small window showing randomly choosen kanjis from the
vocab files.

## dependencies

- gtk+ 3.0

## keys

- 1 show vocab as hiragana/katakana
- 2 show vocab as kanji (default)
- 3 show vocab as heisig interpretion
- 4 show vocab as english translation
- q/space/enter close program

## options

- v <format> vocab file format (default "%i^%h^%k^%r|%e/")
- f <font> choose font (default TakaoMincho)
- s <fontsize> fontsize (default 60)
- h <height> pixel height of popup window (default 200)
- w <width> pixel width of popup window (default 500)
- q <quest> number for kana/kanji/heisig/english like keynumbers

## format

The format string specifies how the vocab file is parsed.
Non matching vocab file lines are ignored. Between the four
vocab parts delimiters must be put. The delimiters dont need
to be the same. After a delimiter or before a command token
arbitrary strings can be inserted. It is not neccessary to
append a delimiter to the english part.

- %i ignore arbitrary string until following delimiter
- %h hiragana/katakana part
- %k kanji part
- %r heisig part
- %e english part
