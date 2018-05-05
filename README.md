# Ved

vktec's nth attempt at creating a code editor

## What?

Ved is a lightweight, modern, graphical text editor. Being graphical
allows it to support interface features that would be impossible on
many terminals (24-bit colour, multiple fonts/sizes, better keypress
detection, etc.). However, users of terminal-based editors such as Vim
will feel right at home in Ved's text-based, modal interface.

## Features

All of the following features are planned, few of them are currently implemented:

- Modal editing. Ved takes inspiration from vim's modes, but tries to
	simplify things by only having 2 modes: command and edit mode. However,
	modes will be able to have sub-modes that modify the behaviour slightly.
- Fancy text rendering. Ved uses Pango for text rendering, which makes
	things look very nice. Eventually, it will also support [elastic
	tabstops][et] as well as automatic detection of alignment, which will
	allow use of variable-width fonts
- Non-blocking IO. When you're doing text editing, you don't want to wait
	around while your hard drive spins up whenever you save. Ved will spawn
	a new concurrent worker to handle writing the data to disk, letting you
	get straight back to editing.
- Hackable. Ved's source code is designed to be concise, well structured
	and easy to understand (I could do with improving my commenting though!)
	In addition, Ved will support a scripting API (possibly through [Wren]),
	allowing plugins to be written.

[et]: http://nickgravgaard.com/elastic-tabstops/
[wren]: http://wren.io/
