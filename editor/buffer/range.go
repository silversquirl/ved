package buffer

type Range interface {
	ByteRange(*Buffer) ByteRange
}

type ByteRange struct { Start, End int }
func (b ByteRange) ByteRange(_ *Buffer) ByteRange { return b }
func (b ByteRange) Coords() (start, end int) {
	return b.Start, b.End
}
