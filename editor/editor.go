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

func New(filename string) (ved *Editor, err error) {
	ved = new(Editor)
	ved.Buf, err = buffer.New(filename)
	if err != nil {
		return
	}

	ved.Modes.Command = command.New()
	// TODO: edit mode through default key handler
	ved.Modes.Edit = command.New()
	ved.Modes.Current = &ved.Modes.Command

	return
}
