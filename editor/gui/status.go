package gui

import (
	"go.vktec.org.uk/gopan"
	"go.vktec.org.uk/gopan/vtkcairo"
	"strings"
)

type StatusBar struct {
	ui *UI
	l gopancairo.CairoLayout
}

func (ui *UI) NewStatusBar() StatusBar {
	l := gopancairo.CreateLayout(ui.win.Cairo())
	l.SetWrap(gopan.WordChar)
	l.SetFontDescription(ui.fonts.Regular)
	return StatusBar{ui, l}
}

func (s *StatusBar) updateText() {
	var parts []string
	switch s.ui.ved.Modes.Current {
	case &s.ui.ved.Modes.Command:
		parts = append(parts, "CMD")
	case &s.ui.ved.Modes.Edit:
		parts = append(parts, "EDIT")
	default:
		panic("Unknown mode")
	}
	s.l.SetText(strings.Join(parts, "\t"))
}

func (s *StatusBar) Resize() {
	s.l.Update()
	w, _ := s.ui.win.Size()
	s.l.SetWidth((w - TextPadding * 2) * gopan.Scale)
}

func (s *StatusBar) Draw(w, h float64) {
	s.updateText()
	_, barh := s.l.PixelSize()
	y := h - float64(barh)

	s.l.Cr.Rectangle(0, y, w, float64(barh))
	s.ui.colours.BarBG.SetCairo(s.l.Cr)
	s.l.Cr.Fill()

	s.l.Cr.MoveTo(TextPadding, y)
	s.ui.colours.Foreground.SetCairo(s.l.Cr)
	s.l.Show()
}
