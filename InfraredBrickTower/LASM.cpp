#include "LASM.h"
#include "stdio.h"

namespace LASM
{
	BOOL ValidateReply(LASMCommandByte commandByte, BYTE* replyBuffer, UINT replyLength)
	{
		BYTE complement = ~commandByte & 0xff;

		// first off, it can't be guaranteed that the typical preamble of 0x55 0xFF 0x00 will be there;
		// occasionally, just a part of it will be there.
		// so, i'll search through the buffer for the complement.
		UINT complementIndex = -1;
		for (UINT i = 0; i < replyLength; i++)
		{
			BYTE b = replyBuffer[i];
			if (b == complement)
			{
				complementIndex = i;
				break;
			}
		}

		if (complementIndex == -1)
		{
			return FALSE;
		}

		// now it's expected that there is a pattern like <complement> <command> <complement> <command>.
		// the presence of that will determine if the reply is good or not.
		return replyBuffer[complementIndex] == complement &&
			replyBuffer[complementIndex + 1] == commandByte &&
			replyBuffer[complementIndex + 2] == complement &&
			replyBuffer[complementIndex + 3] == commandByte;
	}

	CommandData Cmd_PlaySystemSound(SystemSound sound)
	{
		BYTE params[1]{ (BYTE)sound };
		return ComposeCommand(PlaySystemSound, params, 1);
	}

	CommandData ComposeCommand(LASMCommandByte lasmCommand)
	{
		return ComposeCommand(lasmCommand, nullptr, 0);
	}

	CommandData ComposeCommand(LASMCommandByte lasmCommand, BYTE* params, UINT paramsLength)
	{
		CommandData commandData = CommandData(lasmCommand);

		std::shared_ptr<BYTE[]> data = commandData.data;

		UINT index = 0;

		// preamble
		data[index++] = 0x55;
		data[index++] = 0xFF;
		data[index++] = 0x00;

		UINT dataSum = 0;

		// command, reply, and repeat both
		data[index++] = lasmCommand;
		data[index++] = ~lasmCommand;

		dataSum += lasmCommand;

		for (UINT i = 0; i < paramsLength; i++)
		{
			BYTE paramByte = params[i];
			data[index++] = paramByte;
			data[index++] = ~paramByte;

			dataSum += paramByte;
		}

		// checksum for the RCX is just the data sum, so...
		data[index++] = dataSum;
		data[index++] = ~dataSum;

		commandData.dataLength = index;

		return commandData;
	}
}