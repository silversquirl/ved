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

type Keybind struct {
	Key vtk.Key
	Mods vtk.Modifier
}

type CommandMap map[Keybind]CommandNode
type Command func()

type CommandSet struct {
	root CommandMap
	path []Keybind
	Default func(path []Keybind) (newPath []Keybind)
}

func New() CommandSet {
	return CommandSet{make(CommandMap), nil, nil}
}

func (cs CommandSet) get(path []Keybind) CommandNode {
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
	path := append(cs.path, Keybind{ev.Key(), ev.Mods()})
	n := cs.get(path)
	if n == nil {
		if cs.Default == nil {
			cs.cancel()
		} else if p := cs.Default(path); len(p) == 0 {
			cs.cancel()
		} else {
			cs.path = p
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

func parseKeybind(bind string) (keys []Keybind, err error) {
	for i := 0; i < len(bind); {
		switch bind[i] {
		case '<':
			bind = bind[i+1:]
			i = strings.IndexRune(bind, '>')
			if i < 0 {
				return nil, InvalidBindError
			}
			mods := strings.Split(bind[:i], "-")
			mods, key := mods[:len(mods)-1], mods[len(mods)-1]

			k := Keybind{}
			for _, m := range mods {
				k.Mods |= vtk.ModifierFromString(m)
			}

			k.Key = vtk.KeyFromString(key)
			if k.Key == 0 {
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
				keys = append(keys, Keybind{vtk.Key(r), 0})
			} else {
				return nil, InvalidBindError
			}
		}
		bind = bind[i:]
	}
	return
}
