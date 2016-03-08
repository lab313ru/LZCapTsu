//#define ADD_EXPORTS

#include "main.h"
#include "stdlib.h"
#include "string.h"

#if !defined(ADD_EXPORTS)
#include "stdio.h"
#endif

typedef unsigned char byte;
typedef unsigned short ushort;

#define minlen_0 (3)
#define maxlen_0 ((1 << reps_bits_cnt) + 1)
#define maxfrom_bits_0 (16 - reps_bits_cnt)
#define maxfrom_0 (1 << maxfrom_bits_0)
#define maxfrom_mask_0 (maxfrom_0 - 1)

#define minlen_1 (3)
#define maxlen_1 (100)

byte read_byte(byte *input, ushort *readoff)
{
	return (input[(*readoff)++]);
}

void write_byte(byte *output, ushort *writeoff, byte b)
{
	output[(*writeoff)++] = b;
}

ushort read_word(byte *input, ushort *readoff)
{
	ushort retn = read_byte(input, readoff) << 8;
	retn |= read_byte(input, readoff);
	return retn;
}

void write_word(byte *output, ushort *writeoff, ushort w)
{
	write_byte(output, writeoff, w >> 8);
	write_byte(output, writeoff, w & 0xFF);
}

byte read_cmd_bit(byte *input, ushort *readoff, byte *bitscnt, byte *cmd)
{
	(*bitscnt)--;

	if (!*bitscnt) {
		*cmd = read_byte(input, readoff);
		*bitscnt = 8;
	}

	byte retn = *cmd & 1;
	*cmd >>= 1;
	return retn;
}

void write_cmd_bit(byte bit, byte *output, ushort *writeoff, byte *cmdbits, ushort *cmdoff)
{
	if (*cmdbits == 8)
	{
		*cmdbits = 0;
		*cmdoff = (*writeoff)++;
		output[*cmdoff] = 0;
	}

	output[*cmdoff] = (bit << *cmdbits) | output[*cmdoff];
	(*cmdbits)++;
}

ushort do_decompress_0(byte *input, byte *output, ushort out_size)
{
	ushort i = 0, readoff = 0, writeoff = 0;
	byte cmdbits = 0, cmd = 0, bit = 0;
	ushort reps_mask = 0, from_mask = 0;
	ushort reps = 0, from = 0;
	byte reps_bits_cnt = 0;
	byte b = 0;
	ushort w = 0;
	
	cmd = 0;
	cmdbits = 1;

	reps_bits_cnt = read_byte(input, &readoff);

	while (writeoff < out_size)
	{
		bit = read_cmd_bit(input, &readoff, &cmdbits, &cmd);

		if (bit)
		{
			b = read_byte(input, &readoff);
			write_byte(output, &writeoff, b);
		}
		else
		{
			// reps_bits_count = MSB bits in word of reps
			// from = rest of bits

			w = read_word(input, &readoff);
			reps_mask = ((1 << reps_bits_cnt) - 1) << maxfrom_bits_0;
			from_mask = (~reps_mask);
			reps = ((w & reps_mask) >> (16 - reps_bits_cnt)) + 2;
			from = (w & from_mask);

			from = (short)(writeoff - from - 1);

			for (i = 0; i < reps; ++i)
			{
				b = read_byte(output, &from);
				b = ((short)from < 0) ? 0 : b;
				write_byte(output, &writeoff, b);
			}
		}
	}

	return writeoff;
}

ushort do_decompress_0_size(byte *input, ushort out_size)
{
	ushort readoff = 0, writeoff = 0;
	byte cmdbits = 0, cmd = 0, bit = 0;
	ushort reps_mask = 0;
	ushort reps = 0;
	byte reps_bits_cnt = 0;
	ushort w = 0;

	cmd = 0;
	cmdbits = 1;

	reps_bits_cnt = read_byte(input, &readoff);

	while (writeoff < out_size)
	{
		bit = read_cmd_bit(input, &readoff, &cmdbits, &cmd);

		if (bit)
		{
			readoff++;
			writeoff++;
		}
		else
		{
			readoff += 2;
			w = read_word(input, &readoff);
			reps_mask = ((1 << reps_bits_cnt) - 1) << (16 - reps_bits_cnt);
			reps = ((w & reps_mask) >> (16 - reps_bits_cnt)) + 2;

			writeoff += reps;
		}
	}

	return readoff;
}

ushort do_decompress_1_byte(byte *input, byte *output, ushort in_size)
{
	ushort readoff = 0, writeoff = 0;
	byte i = 0, b = 0, reps = 0;

	while (readoff < in_size)
	{
		b = read_byte(input, &readoff);

		if (b == input[readoff])
		{
			readoff++;
			reps = read_byte(input, &readoff) + 2;
		}
		else
		{
			reps = 1;
		}

		for (i = 0; i < reps; ++i)
		{
			write_byte(output, &writeoff, b);
		}
	}

	return writeoff;
}

ushort do_decompress_1_byte_size(byte *input, ushort in_size)
{
	ushort readoff = 0;
	byte b = 0;

	while (readoff < in_size)
	{
		b = read_byte(input, &readoff);

		if (b == input[readoff])
		{
			readoff++;
			readoff++;
		}
	}

	return readoff;
}

ushort do_decompress_1_word(byte *input, byte *output, ushort in_size)
{
	ushort w = 0, readoff = 0, writeoff = 0;
	byte i = 0, reps = 0;

	while (readoff < in_size)
	{
		w = read_word(input, &readoff);

		if (
			(w >> 0x8) == input[readoff + 0] &&
			(w & 0xFF) == input[readoff + 1]
			)
		{
			readoff += 2;
			reps = read_byte(input, &readoff) + 2;
		}
		else
		{
			reps = 1;
		}

		for (i = 0; i < reps; ++i)
		{
			write_word(output, &writeoff, w);
		}
	}

	return writeoff;
}

ushort do_decompress_1_word_size(byte *input, ushort in_size)
{
	ushort w = 0, readoff = 0;

	while (readoff < in_size)
	{
		w = read_word(input, &readoff);

		if (
			(w >> 0x8) == input[readoff + 0] &&
			(w & 0xFF) == input[readoff + 1]
			)
		{
			readoff += 2;
			readoff++;
		}
	}

	return readoff;
}

ushort do_decompress_1_copy(byte *input, byte *output)
{
	ushort readoff = 0;
	short size = (short)read_word(input, &readoff);
	size = (size < 0) ? 3 : size;

	memcpy(output, &input[readoff], size);

	return size;
}

ushort do_decompress_1_copy_size(byte *input)
{
	ushort readoff = 0;
	short size = (short)read_word(input, &readoff);
	return (size < 0) ? 3 : size;
}

ushort ADDCALL decompress(byte *input, byte *output)
{
	ushort bit = 0, readoff = 0, out_size = 0;
	byte method = 0;

	readoff = 0;
	out_size = read_word(input, &readoff);

	bit = (out_size & 0x8000);
	out_size &= ~0x8000;

	if (bit)
	{
		method = read_byte(input, &readoff) & 3;

		switch (method)
		{
		case 0: return do_decompress_1_byte(&input[readoff], output, out_size);
		case 1: return do_decompress_1_word(&input[readoff], output, out_size);
		case 2:
		case 3: return do_decompress_1_copy(&input[readoff], output);
		}
	}

	return do_decompress_0(&input[readoff], output, out_size);
}

ushort ADDCALL compressed_size(byte *input)
{
	ushort bit = 0, readoff = 0, out_size = 0;
	byte method = 0;

	readoff = 0;
	out_size = read_word(input, &readoff);

	bit = (out_size & 0x8000);
	out_size &= ~0x8000;

	if (bit)
	{
		method = read_byte(input, &readoff) & 3;

		switch (method)
		{
		case 0: return do_decompress_1_byte_size(&input[readoff], out_size);
		case 1: return do_decompress_1_word_size(&input[readoff], out_size);
		case 2:
		case 3: return do_decompress_1_copy_size(&input[readoff]);
		}
	}

	return do_decompress_0_size(&input[readoff], out_size);
}

void find_xx_matches(byte *input, ushort readoff, ushort size, ushort *reps, ushort *from, byte reps_bits_cnt)
{
	*reps = 1;
	*from = 0;

	while (
		readoff + *reps < size &&
		readoff + *reps < maxfrom_0 &&
		*reps < maxlen_0 &&
		input[readoff + *reps] == input[readoff]
		)
	{
		(*reps)++;
	}
}

void find_matches_0(byte *input, ushort readoff, ushort size, ushort *reps, ushort *from, byte reps_bits_cnt)
{
	ushort len = 0;
	ushort pos = 0;

	*from = 0;
	*reps = 1;

	while (
		pos < readoff &&
		readoff < size
		)
	{
		len = 0;

		while (
			(pos + 1) < readoff &&
			(readoff - pos) < maxfrom_0 &&
			input[readoff] != input[pos]
			)
		{
			pos++;
		}

		while (
			(pos + 1) < readoff &&
			readoff + len < size &&
			len < maxlen_0 &&
			(readoff - pos) < maxfrom_0 &&
			input[readoff + len] == input[pos + len]
			)
		{
			len++;
		}

		if (len >= *reps && len >= minlen_0)
		{
			*reps = len;
			*from = pos;
		}

		pos++;
	}
}

byte calc_word_bits(ushort value)
{
	for (byte i = 15; i > 0; --i)
	{
		if (value & (1 << i))
		{
			return (i + 1);
		}
	}

	return 0;
}

ushort do_compress_0(byte *input, byte *output, ushort size)
{
	ushort readoff = 0, min_writeoff = 0, writeoff = 0, cmdoff = 0;
	byte cmdbits = 0;
	byte b = 0;
	ushort w = 0;
	byte min_reps_bits_cnt = 0, reps_bits_cnt = 0;
	ushort reps = 0, from = 0;

	write_byte(output, &writeoff, 0); // reps_bits_cnt

	reps_bits_cnt = 8;
	while (reps_bits_cnt > 0)
	{
		readoff = 0;
		writeoff = 2;
		cmdoff = 1;
		cmdbits = 0;
		output[cmdoff] = 0;

		while (readoff < size)
		{
			find_xx_matches(input, readoff, size, &reps, &from, reps_bits_cnt);

			if (
				readoff >= maxfrom_0 &&
				reps >= minlen_0 &&
				input[readoff] == 0
				)
			{
				write_cmd_bit(0, output, &writeoff, &cmdbits, &cmdoff);
				writeoff += 2;

				readoff += reps;
			}
			else
			{
				find_matches_0(input, readoff, size, &reps, &from, reps_bits_cnt);

				if (reps >= minlen_0)
				{
					write_cmd_bit(0, output, &writeoff, &cmdbits, &cmdoff);
					writeoff += 2;

					readoff += reps;
				}
				else
				{
					write_cmd_bit(1, output, &writeoff, &cmdbits, &cmdoff);

					readoff++;
					writeoff++;
				}
			}
		}

		if (
			min_writeoff == 0 ||
			min_writeoff > writeoff
			)
		{
			min_writeoff = writeoff;
			min_reps_bits_cnt = reps_bits_cnt;
		}

		reps_bits_cnt--;
	}

	readoff = 0;
	writeoff = 2;
	cmdoff = 1;
	cmdbits = 0;
	output[cmdoff] = 0;

	reps_bits_cnt = min_reps_bits_cnt;
	while (readoff < size)
	{
		find_xx_matches(input, readoff, size, &reps, &from, reps_bits_cnt);

		if (
			readoff >= maxfrom_0 &&
			reps >= minlen_0 &&
			input[readoff] == 0
			)
		{
			write_cmd_bit(0, output, &writeoff, &cmdbits, &cmdoff);

			w = ((readoff - from - 1) * ((from == 0) ? -1 : 1)) & maxfrom_mask_0;
			w |= ((reps - 2) << maxfrom_bits_0);
			write_word(output, &writeoff, w);

			readoff += reps;
		}
		else
		{
			find_matches_0(input, readoff, size, &reps, &from, reps_bits_cnt);

			if (reps >= minlen_0)
			{
				write_cmd_bit(0, output, &writeoff, &cmdbits, &cmdoff);

				w = ((readoff - from - 1) * ((from == 0) ? -1 : 1)) & maxfrom_mask_0;
				w |= ((reps - 2) << maxfrom_bits_0);
				write_word(output, &writeoff, w);

				readoff += reps;
			}
			else
			{
				write_cmd_bit(1, output, &writeoff, &cmdbits, &cmdoff);

				b = read_byte(input, &readoff);
				write_byte(output, &writeoff, b);
			}
		}
	}

	cmdbits = 8;
	write_cmd_bit(0, output, &writeoff, &cmdbits, &cmdoff);

	readoff = 0; // as writeoff
	write_byte(output, &readoff, reps_bits_cnt);

	return writeoff;
}

void find_matches_1_byte(byte *input, ushort readoff, ushort size, ushort *reps)
{
	*reps = 1;

	while (
		readoff + *reps < size &&
		*reps < maxlen_1 &&
		input[readoff + *reps] == input[readoff]
		)
	{
		(*reps)++;
	}
}

ushort do_compress_1_byte(byte *input, byte *output, ushort size)
{
	ushort readoff = 0, writeoff = 0;
	ushort reps = 0;
	byte b = 0;

	while (readoff < size)
	{
		find_matches_1_byte(input, readoff, size, &reps);

		if (reps >= minlen_1)
		{
			write_byte(output, &writeoff, input[readoff]);
			write_byte(output, &writeoff, input[readoff]);

			write_byte(output, &writeoff, (byte)(reps - 2));

			readoff += reps;
		}
		else
		{
			b = read_byte(input, &readoff);
			write_byte(output, &writeoff, b);
		}
	}

	return writeoff;
}

void find_matches_1_word(byte *input, ushort readoff, ushort size, ushort *reps)
{
	*reps = 1;

	while (
		readoff + *reps < size &&
		*reps < maxlen_1 &&
		input[readoff + *reps] == input[readoff] &&
		input[readoff + *reps + 1] == input[readoff + 1]
		)
	{
		(*reps)++;
	}
}

ushort do_compress_1_word(byte *input, byte *output, ushort size)
{
	ushort readoff = 0, writeoff = 0;
	ushort reps = 0;
	ushort w = 0;

	while (readoff < size)
	{
		find_matches_1_word(input, readoff, size, &reps);

		if (reps >= minlen_1)
		{
			write_word(output, &writeoff, (input[readoff] << 8) | (input[readoff + 1]));
			write_word(output, &writeoff, (input[readoff] << 8) | (input[readoff + 1]));

			write_byte(output, &writeoff, (byte)(reps - 2));

			readoff += reps;
		}
		else
		{
			w = read_word(input, &readoff);
			write_word(output, &writeoff, w);
		}
	}

	return writeoff;
}

ushort do_compress_1_copy(byte *input, byte *output, ushort size)
{
	ushort writeoff = 0;

	write_word(output, &writeoff, size);
	memcpy(&output[writeoff], input, size);

	return size;
}

ushort ADDCALL compress(byte *input, byte *output, ushort size)
{
	byte mode = 0;
	ushort min_size = 0, dest_size = 0;
	ushort writeoff = 0;

	dest_size = do_compress_0(input, output, size);
	min_size = dest_size;
	mode = 0;

	dest_size = do_compress_1_byte(input, output, size);
	if (
		min_size == 0 ||
		min_size > dest_size
		)
	{
		min_size = dest_size;
		mode = 1;
	}

	if ((size & 1) == 0)
	{
		dest_size = do_compress_1_word(input, output, size);
		if (
			min_size == 0 ||
			min_size > dest_size
			)
		{
			min_size = dest_size;
			mode = 2;
		}
	}

	if (size >= 3)
	{
		dest_size = do_compress_1_copy(input, output, size);
		if (
			min_size == 0 ||
			min_size > dest_size
			)
		{
			min_size = dest_size;
			mode = 3;
		}
	}

	switch (mode)
	{
	case 1:
	case 2:
	case 3:
	{
		write_word(output, &writeoff, (size & 0x8000));

		write_byte(output, &writeoff, mode);
		if (mode == 1)
		{
			dest_size = do_compress_1_byte(input, &output[writeoff], size);
		}
		else if (mode == 2)
		{
			dest_size = do_compress_1_word(input, &output[writeoff], size);
		}
		else // (mode == 3)
		{
			dest_size = do_compress_1_copy(input, &output[writeoff], size);
		}
	} break;
	default:
	{
		write_word(output, &writeoff, size);
		dest_size = do_compress_0(input, &output[writeoff], size);
	} break;
	}

	return (dest_size & 1) ? dest_size + 1 : dest_size;
}

#if !defined (ADD_EXPORTS)
int main(int argc, char *argv[])
{
	byte *input, *output;

	FILE *inf = fopen(argv[1], "rb");

	input = (byte *)malloc(0x10000);
	output = (byte *)malloc(0x10000);

	char mode = (argv[3][0]);

	if (mode == 'd')
	{
		long offset = strtol(argv[4], NULL, 16);
		fseek(inf, offset, SEEK_SET);
	}

	fread(&input[0], 1, 0x10000, inf);

	int dest_size;
	if (mode == 'd')
	{
		dest_size = decompress(input, output);
	}
	else
	{
		fseek(inf, 0, SEEK_END);
		int dec_size = ftell(inf);

		dest_size = compress(input, output, (ushort)dec_size);
	}

	if (dest_size != 0)
	{
		FILE *outf = fopen(argv[2], "wb");
		fwrite(&output[0], 1, dest_size, outf);
		fclose(outf);
	}

	fclose(inf);

	free(input);
	free(output);

	return 0;
}
#endif
