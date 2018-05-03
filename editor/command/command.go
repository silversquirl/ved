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
			k := vtk.KeyFromString(bind[:i])
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
