package gui

import "go.vktec.org.uk/vtk/cairo"

type Colour struct {
	R, G, B, A float64
}

func (c Colour) SetCairo(cr cairo.Cairo) {
	cr.SetSourceRGBA(c.R, c.G, c.B, c.A)
}

type ColourScheme struct {
	Foreground, Background, Line, BarBG Colour
}
