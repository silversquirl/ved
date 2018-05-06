package buffer

import (
	"io"
	"os"
	"runtime"
)

type Buffer struct {
	r interface {
		io.ReaderAt
		io.Closer
	}
	buf      []byte
	end      int
	eof      bool
	DamageCB func()

	Cursor int
}

func New(filename string) (buf Buffer, err error) {
	buf = Buffer{}
	buf.r, err = os.OpenFile(filename, os.O_RDWR|os.O_CREATE, 0666)
	runtime.SetFinalizer(&buf, func(buf *Buffer) { buf.r.Close() })
	return
}

func (buf Buffer) AtEOF() bool {
	return buf.eof
}

func (buf *Buffer) ExtendView(step int) error {
	off := len(buf.buf)
	tmp := make([]byte, step)
	n, err := buf.r.ReadAt(tmp, int64(off))
	if err == io.EOF {
		buf.eof = true
	} else if err != nil {
		return err
	}
	buf.buf = append(buf.buf, tmp[:n]...)
	buf.end += n
	return nil
}

func (buf Buffer) Text() string {
	return string(buf.buf)
}

// TODO: editing functions. Make sure they call DamageCB
func (buf *Buffer) Insert(text string) {
	n := len(text)
	if n == 1 {
		buf.InsertChar(text[0])
		return
	}

	buf.buf = append(buf.buf, make([]byte, n)...)
	copy(buf.buf[buf.Cursor+n:], buf.buf[buf.Cursor:])
	copy(buf.buf[buf.Cursor:], text)
	buf.Cursor += n

	if buf.DamageCB != nil {
		buf.DamageCB()
	}
}

// Special case which is slightly more efficient
func (buf *Buffer) InsertChar(ch byte) {
	buf.buf = append(buf.buf, 0)
	copy(buf.buf[buf.Cursor+1:], buf.buf[buf.Cursor:])
	buf.buf[buf.Cursor] = ch
	buf.Cursor++

	if buf.DamageCB != nil {
		buf.DamageCB()
	}
}

func (buf *Buffer) Delete(n int) {
	if n < 0 {
		n = -n
		if buf.Cursor < n {
			return
		}
		buf.Cursor -= n
	}
	if buf.Cursor + n > len(buf.buf) {
		return
	}
	buf.buf = append(buf.buf[:buf.Cursor], buf.buf[buf.Cursor+n:]...)

	if buf.DamageCB != nil {
		buf.DamageCB()
	}
}
