// TODO: modifiers (eg. <Ctrl-A>, <Super-@>, etc.)
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
	root CommandMap
	path []vtk.Key
	Default func(path []vtk.Key) (needMore bool)
}

func New() CommandSet {
	m := make(CommandMap)
	return CommandSet{m, nil, nil}
}

func (cs CommandSet) get(path []vtk.Key) CommandNode {
	cur := CommandNode(cs.root)
	for _, k := range path {
		if m, ok := cur.(CommandMap); ok {
			cur = m[k]
		} else {
			return nil
		}
	}
	return cur
}

func (cs *CommandSet) HandleKey(ev vtk.KeyEvent) {
	path := append(cs.path, ev.Key())
	n := cs.get(path)
	if n == nil {
		if cs.Default != nil && cs.Default(path) {
			cs.path = path
		} else {
			cs.cancel()
		}
		return
	}

	switch n := n.(type) {
	case CommandMap:
		cs.path = path
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
	// Unlike setting to nil, this preserves the allocated memory
	cs.path = cs.path[:0]
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
			if ' ' <= r && r <= '~' {
				keys = append(keys, vtk.Key(r))
			} else {
				return nil, InvalidBindError
			}
		}
		bind = bind[i:]
	}
	return
}
