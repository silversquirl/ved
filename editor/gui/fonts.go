package gui

import "go.vktec.org.uk/gopan"

type Fonts struct {
	Regular gopan.FontDescription
}

func NewFonts() Fonts {
	// TODO: configuration
	// TODO: more types of font (bold, italic, etc)
	return Fonts {
		gopan.FontDescriptionFromString("Helvetica 11"),
	}
}
