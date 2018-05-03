// TODO: modifiers (eg. <Ctrl-A>, <Super-@>, etc.)
// TODO: edit mode. Possible approach through wildcard commands?
package command

import (
	"errors"
	"go.vktec.org.uk/vtk"
	"strings"
	"unicode/utf8"
)

var (
	CollisionError = errors.New("Keybind collision")
	InvalidBindError = errors.New("Invalid keybind string")
)

type CommandNode interface {
	commandNode()
}

func (_ CommandMap) commandNode() {}
func (_ Command) commandNode()    {}

type CommandMap map[vtk.Key]CommandNode
type Command func()

type CommandSet struct {
	root, cur CommandMap
}

func New() CommandSet {
	m := make(CommandMap)
	return CommandSet{m, m}
}

func (cs *CommandSet) HandleKey(ev vtk.KeyEvent) {
	n := cs.cur[ev.Key()]
	switch n := n.(type) {
	case CommandMap:
		cs.cur = n
	case Command:
		n()
		cs.cancel()
	}
}

func (cs *CommandSet) Add(bind string, cmd func()) error {
	keys, err := parseKeybind(bind)
	if err != nil {
		return err
	}

	cur := cs.root
	for {
		k, keys := keys[0], keys[1:]
		if len(keys) == 0 {
			cur[k] = Command(cmd)
			return nil
		}

		next := cur[k]
		if next == nil {
			next = make(CommandMap)
			cur[k] = next
		}
		switch next := next.(type) {
		case Command:
			return CollisionError
		case CommandMap:
			cur = next
		}
	}
	return nil
}

func (cs *CommandSet) cancel() {
	cs.cur = cs.root
}

func parseKeybind(bind string) (keys []vtk.Key, err error) {
	for i := 0; i < len(bind); {
		switch bind[i] {
		case '<':
			bind = bind[i+1:]
			i = strings.IndexRune(bind, '>')
			if i < 0 {
				return nil, InvalidBindError
			}
			k := parseKey(bind[:i])
			if k == 0 {
				return nil, InvalidBindError
			} else {
				keys = append(keys, k)
			}

		case '\\':
			i++
			fallthrough
		default:
			r, n := utf8.DecodeRuneInString(bind[i:])
			i += n
			if ' ' <= ch && ch <= '~' {
				keys = append(keys, vtk.Key(r))
			} else {
				return nil, InvalidBindError
			}
		}
		bind = bind[i:]
	}
	return
}

func parseKey(key string) vtk.Key {
	// FIXME: This should be done in vtk
	switch strings.ToLower(key) {
	case "backspace":
		return vtk.Backspace
	case "tab":
		return vtk.Tab
	case "return":
		return vtk.Return
	case "escape":
		return vtk.Escape
	case "space":
		return vtk.Space
	case "delete":
		return vtk.Delete
	case "insert":
		return vtk.Insert
	case "pageup":
		return vtk.PageUp
	case "pagedown":
		return vtk.PageDown
	case "home":
		return vtk.Home
	case "end":
		return vtk.End
	case "up":
		return vtk.Up
	case "down":
		return vtk.Down
	case "left":
		return vtk.Left
	case "right":
		return vtk.Right
	default:
		return 0
	}
}
