package editor

import (
	"./buffer"
	"./command"
)

type Editor struct {
	Buf   buffer.Buffer
	Modes struct {
		Command, Edit command.CommandSet
		Current       *command.CommandSet
	}
}

func (ved *Editor) editHandler(path []command.Keybind) []command.Keybind {
	k, rest := path[0], path[1:]
	if k.Mods == 0 && (' ' <= k.Key && k.Key <= '~'  || k.Key == '\n' || k.Key == '\t') {
		ved.Buf.InsertChar(byte(k.Key))
	}
	return rest
}

func New(filename string) (ved *Editor, err error) {
	ved = new(Editor)
	ved.Buf, err = buffer.New(filename)
	if err != nil {
		return
	}

	ved.Modes.Command = command.New()
	ved.Modes.Edit = command.New()
	ved.Modes.Edit.Default = ved.editHandler
	ved.Modes.Current = &ved.Modes.Command

	return
}
