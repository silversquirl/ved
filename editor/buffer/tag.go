package buffer

import (
	"image/color"
	"go.vktec.org.uk/gopan"
)

type TagMask int

const (
	SetForeground TagMask = 1 << iota
	SetBackground
	SetStyle
	SetFont
)

type Style int

const (
	// Light and Bold should not both be applied simultaneously
	Bold Style = 1 << iota
	Light
	Italic
	Underline
)

type Tag struct {
	Mask TagMask
	Foreground, Background color.Color
	Style Style
	Font gopan.FontDescription
}
