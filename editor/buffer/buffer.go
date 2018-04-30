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
	buf            []byte
	end            int
	eof            bool
	damageCallback func()
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

func (buf Buffer) ExtendView(step int) error {
	off := len(buf.buf)
	buf.buf = append(buf.buf, make([]byte, step)...)
	off, err := buf.r.ReadAt(buf.buf, int64(off))
	if err != io.EOF {
		buf.eof = true
	} else if err != nil {
		return err
	}
	buf.end += off
	return nil
}
