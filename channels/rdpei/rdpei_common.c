/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Input Virtual Channel Extension
 *
 * Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 * Copyright 2014 David Fort <contact@hardening-consulting.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <freerdp/config.h>

#include <winpr/crt.h>
#include <winpr/stream.h>

#include "rdpei_common.h"

#include <freerdp/log.h>

#define TAG FREERDP_TAG("channels.rdpei.common")

BOOL rdpei_read_2byte_unsigned(wStream* s, UINT16* value)
{
	BYTE byte = 0;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, 1))
		return FALSE;

	Stream_Read_UINT8(s, byte);

	if (byte & 0x80)
	{
		if (!Stream_CheckAndLogRequiredLength(TAG, s, 1))
			return FALSE;

		*value = (byte & 0x7F) << 8;
		Stream_Read_UINT8(s, byte);
		*value |= byte;
	}
	else
	{
		*value = (byte & 0x7F);
	}

	return TRUE;
}

BOOL rdpei_write_2byte_unsigned(wStream* s, UINT16 value)
{
	BYTE byte = 0;

	if (!Stream_EnsureRemainingCapacity(s, 2))
		return FALSE;

	if (value > 0x7FFF)
		return FALSE;

	if (value >= 0x7F)
	{
		byte = ((value & 0x7F00) >> 8);
		Stream_Write_UINT8(s, byte | 0x80);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else
	{
		byte = (value & 0x7F);
		Stream_Write_UINT8(s, byte);
	}

	return TRUE;
}

BOOL rdpei_read_2byte_signed(wStream* s, INT16* value)
{
	BYTE byte = 0;
	BOOL negative = 0;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, 1))
		return FALSE;

	Stream_Read_UINT8(s, byte);

	negative = (byte & 0x40) ? TRUE : FALSE;

	*value = (byte & 0x3F);

	if (byte & 0x80)
	{
		if (!Stream_CheckAndLogRequiredLength(TAG, s, 1))
			return FALSE;

		Stream_Read_UINT8(s, byte);
		*value = (*value << 8) | byte;
	}

	if (negative)
		*value *= -1;

	return TRUE;
}

BOOL rdpei_write_2byte_signed(wStream* s, INT16 value)
{
	BYTE byte = 0;
	BOOL negative = FALSE;

	if (!Stream_EnsureRemainingCapacity(s, 2))
		return FALSE;

	if (value < 0)
	{
		negative = TRUE;
		value *= -1;
	}

	if (value > 0x3FFF)
		return FALSE;

	if (value >= 0x3F)
	{
		byte = ((value & 0x3F00) >> 8);

		if (negative)
			byte |= 0x40;

		Stream_Write_UINT8(s, byte | 0x80);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else
	{
		byte = (value & 0x3F);

		if (negative)
			byte |= 0x40;

		Stream_Write_UINT8(s, byte);
	}

	return TRUE;
}

BOOL rdpei_read_4byte_unsigned(wStream* s, UINT32* value)
{
	BYTE byte = 0;
	BYTE count = 0;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, 1))
		return FALSE;

	Stream_Read_UINT8(s, byte);

	count = (byte & 0xC0) >> 6;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, count))
		return FALSE;

	switch (count)
	{
		case 0:
			*value = (byte & 0x3F);
			break;

		case 1:
			*value = (byte & 0x3F) << 8;
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 2:
			*value = (byte & 0x3F) << 16;
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 3:
			*value = (byte & 0x3F) << 24;
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 16);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		default:
			break;
	}

	return TRUE;
}

BOOL rdpei_write_4byte_unsigned(wStream* s, UINT32 value)
{
	BYTE byte = 0;

	if (!Stream_EnsureRemainingCapacity(s, 4))
		return FALSE;

	if (value <= 0x3FUL)
	{
		Stream_Write_UINT8(s, value);
	}
	else if (value <= 0x3FFFUL)
	{
		byte = (value >> 8) & 0x3F;
		Stream_Write_UINT8(s, byte | 0x40);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x3FFFFFUL)
	{
		byte = (value >> 16) & 0x3F;
		Stream_Write_UINT8(s, byte | 0x80);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x3FFFFFFFUL)
	{
		byte = (value >> 24) & 0x3F;
		Stream_Write_UINT8(s, byte | 0xC0);
		byte = (value >> 16) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL rdpei_read_4byte_signed(wStream* s, INT32* value)
{
	BYTE byte = 0;
	BYTE count = 0;
	BOOL negative = 0;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, 1))
		return FALSE;

	Stream_Read_UINT8(s, byte);

	count = (byte & 0xC0) >> 6;
	negative = (byte & 0x20) ? TRUE : FALSE;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, count))
		return FALSE;

	switch (count)
	{
		case 0:
			*value = (byte & 0x1F);
			break;

		case 1:
			*value = (byte & 0x1F) << 8;
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 2:
			*value = (byte & 0x1F) << 16;
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 3:
			*value = (byte & 0x1F) << 24;
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 16);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		default:
			break;
	}

	if (negative)
		*value *= -1;

	return TRUE;
}

BOOL rdpei_write_4byte_signed(wStream* s, INT32 value)
{
	BYTE byte = 0;
	BOOL negative = FALSE;

	if (!Stream_EnsureRemainingCapacity(s, 4))
		return FALSE;

	if (value < 0)
	{
		negative = TRUE;
		value *= -1;
	}

	if (value <= 0x1FL)
	{
		byte = value & 0x1F;

		if (negative)
			byte |= 0x20;

		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFL)
	{
		byte = (value >> 8) & 0x1F;

		if (negative)
			byte |= 0x20;

		Stream_Write_UINT8(s, byte | 0x40);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFL)
	{
		byte = (value >> 16) & 0x1F;

		if (negative)
			byte |= 0x20;

		Stream_Write_UINT8(s, byte | 0x80);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFFFL)
	{
		byte = (value >> 24) & 0x1F;

		if (negative)
			byte |= 0x20;

		Stream_Write_UINT8(s, byte | 0xC0);
		byte = (value >> 16) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL rdpei_read_8byte_unsigned(wStream* s, UINT64* value)
{
	UINT64 byte = 0;
	BYTE count = 0;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, 1))
		return FALSE;

	Stream_Read_UINT8(s, byte);

	count = (byte & 0xE0) >> 5;

	if (!Stream_CheckAndLogRequiredLength(TAG, s, count))
		return FALSE;

	switch (count)
	{
		case 0:
			*value = (byte & 0x1F);
			break;

		case 1:
			*value = (byte & 0x1FU) << 8U;
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 2:
			*value = (byte & 0x1FU) << 16U;
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8U);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 3:
			*value = (byte & 0x1FU) << 24U;
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 16U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8U);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 4:
			*value = ((byte & 0x1FU)) << 32U;
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 24U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 16U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8U);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 5:
			*value = ((byte & 0x1FU)) << 40U;
			Stream_Read_UINT8(s, byte);
			*value |= ((byte) << 32U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 24U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 16U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8U);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 6:
			*value = ((byte & 0x1FU)) << 48U;
			Stream_Read_UINT8(s, byte);
			*value |= ((byte) << 40U);
			Stream_Read_UINT8(s, byte);
			*value |= ((byte) << 32U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 24U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 16U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8U);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		case 7:
			*value = ((byte & 0x1FU)) << 56U;
			Stream_Read_UINT8(s, byte);
			*value |= ((byte) << 48U);
			Stream_Read_UINT8(s, byte);
			*value |= ((byte) << 40U);
			Stream_Read_UINT8(s, byte);
			*value |= ((byte) << 32U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 24U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 16U);
			Stream_Read_UINT8(s, byte);
			*value |= (byte << 8U);
			Stream_Read_UINT8(s, byte);
			*value |= byte;
			break;

		default:
			break;
	}

	return TRUE;
}

BOOL rdpei_write_8byte_unsigned(wStream* s, UINT64 value)
{
	BYTE byte = 0;

	if (!Stream_EnsureRemainingCapacity(s, 8))
		return FALSE;

	if (value <= 0x1FULL)
	{
		byte = value & 0x1F;
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFULL)
	{
		byte = (value >> 8) & 0x1F;
		byte |= (1 << 5);
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFULL)
	{
		byte = (value >> 16) & 0x1F;
		byte |= (2 << 5);
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFFFULL)
	{
		byte = (value >> 24) & 0x1F;
		byte |= (3 << 5);
		Stream_Write_UINT8(s, byte);
		byte = (value >> 16) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFFFFFULL)
	{
		byte = (value >> 32) & 0x1F;
		byte |= (4 << 5);
		Stream_Write_UINT8(s, byte);
		byte = (value >> 24) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 16) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFFFFFFFULL)
	{
		byte = (value >> 40) & 0x1F;
		byte |= (5 << 5);
		Stream_Write_UINT8(s, byte);
		byte = (value >> 32) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 24) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 16) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFFFFFFFFFULL)
	{
		byte = (value >> 48) & 0x1F;
		byte |= (6 << 5);
		Stream_Write_UINT8(s, byte);
		byte = (value >> 40) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 32) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 24) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 16) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else if (value <= 0x1FFFFFFFFFFFFFFFULL)
	{
		byte = (value >> 56) & 0x1F;
		byte |= (7 << 5);
		Stream_Write_UINT8(s, byte);
		byte = (value >> 48) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 40) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 32) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 24) & 0x1F;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 16) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value >> 8) & 0xFF;
		Stream_Write_UINT8(s, byte);
		byte = (value & 0xFF);
		Stream_Write_UINT8(s, byte);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

void touch_event_reset(RDPINPUT_TOUCH_EVENT* event)
{
	UINT16 i = 0;

	for (i = 0; i < event->frameCount; i++)
		touch_frame_reset(&event->frames[i]);

	free(event->frames);
	event->frames = NULL;
	event->frameCount = 0;
}

void touch_frame_reset(RDPINPUT_TOUCH_FRAME* frame)
{
	free(frame->contacts);
	frame->contacts = NULL;
	frame->contactCount = 0;
}

void pen_event_reset(RDPINPUT_PEN_EVENT* event)
{
	UINT16 i = 0;

	for (i = 0; i < event->frameCount; i++)
		pen_frame_reset(&event->frames[i]);

	free(event->frames);
	event->frames = NULL;
	event->frameCount = 0;
}

void pen_frame_reset(RDPINPUT_PEN_FRAME* frame)
{
	free(frame->contacts);
	frame->contacts = NULL;
	frame->contactCount = 0;
}
