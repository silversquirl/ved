package command

import (
	"go.vktec.org.uk/vtk"
)

type CommandNode interface {
	commandNode()
}

func (_ CommandSet) commandNode() {}
func (_ Command) commandNode()    {}

type CommandSet map[vtk.Key]CommandNode
type Command func()

func New() CommandSet {
	return make(CommandSet)
}

func (cs CommandSet) HandleKey(ev vtk.KeyEvent) {
	// TODO
}

func (cs CommandSet) Add(bind string, cmd func()) error {
	// TODO
	return nil
}
