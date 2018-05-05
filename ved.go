package main

import (
	"./editor"
	"./editor/gui"
)

func main() {
	ved, err := editor.New("editor/gui/ui.go")
	if err != nil {
		panic(err) // TODO: don't panic
	}

	ui, err := gui.New(ved)
	if err != nil {
		panic(err) // TODO: don't panic
	}

	ved.Modes.Command.Add("<Escape>", func() {
		ui.Quit()
	})

	ved.Modes.Command.Add("i", func() {
		ved.Modes.Current = &ved.Modes.Edit
		ui.Redraw()
	})

	ved.Modes.Edit.Add("<Escape>", func() {
		ved.Modes.Current = &ved.Modes.Command
		ui.Redraw()
	})

	ui.Mainloop()
}
