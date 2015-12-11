
/*
* Copyright: (C) 2010 RobotCub Consortium
* Author: Paul Fitzpatrick
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/
#include <yarp/os/all.h>
#include <stdio.h>

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "klairconfig.h"
#include "klairrpc_h.h"

#include "wincon.h"		// for console text operations

using namespace yarp::os;


// some sample facial expressions

float	expression_middling[EXPRESS_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // neutral

float	expression_happy[EXPRESS_SIZE] = { 1, 1, 1, 0, 0, 0, 0.5f, 0.5f, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 }; // happiness

float	expression_confused[EXPRESS_SIZE] = { 0, 1, 1, 0, -.2f, -.2f, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // confusion

float	expression_bored[EXPRESS_SIZE] = { 1, 0, 0, 0, .2f, .2f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // boredom

float	expression_angry[EXPRESS_SIZE] = { 0, 0, 0, 2, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 }; // anger

float	expression_sad[EXPRESS_SIZE] = { 2, 0, 0, 0, .2f, .2f, 0, 0, 0, 0, 0, 0, 0, 0, 0, .5f, 2, 0, 0, .25f, .25f, 0, 0 }; // sadness 1

float	expression_sad2[EXPRESS_SIZE] = { 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, .25f, .25f, 0, 0 }; // sadness 2



// some sample vowel configurations

float	avowel[VTRACT_SIZE] = { 0.0f, 1.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, -0.3f, 0.0f, 0.5f };

float	ivowel[VTRACT_SIZE] = { 0.23f, -0.3f, 0.4f, 0.0f, -0.06f, -0.63f, 0.0f, 0.0f, 0.0f, -0.3f, 0.0f, 0.5f };

float	uvowel[VTRACT_SIZE] = { 0.23f, 0.5f, 0.57f, 0.0f, -0.23f, 0.87f, 0.0f, 0.0f, 0.0f, -0.3f, 0.0f, 0.5f };

float	relax[VTRACT_SIZE] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.5f };


int main(int argc, char * argv[])
{

	// Set up YARP
	Network yarp;
	// Make two ports called /hello/in and /hello/out
	// We'll send "Bottles" (a simple nested list container) between these ports
	BufferedPort<Bottle> inExpressionPort, inVocalTractPort, outPort;
	bool ok = inExpressionPort.open("/klair_yarp_client/expression:i");
	ok = ok && inVocalTractPort.open("/klair_yarp_client/vocal_tract:i");
	ok = ok && outPort.open("/klair_yarp_client/out");
	if (!ok) {
		fprintf(stderr, "Failed to create ports.\n");
		fprintf(stderr, "Maybe you need to start a nameserver (run 'yarpserver')\n");
		return 1;
	}

	//while (!yarp.connect("/expression:o", inPort.getName())) {
	//	printf("Connecting...");
	//}

	// RPC interface
	unsigned char * pszUuid = NULL;
	unsigned char * pszProtocolSequence = (unsigned char *)"ncacn_np";
	unsigned char * pszNetworkAddress = NULL;
	unsigned char * pszEndpoint = (unsigned char *)"\\pipe\\klairserver";
	unsigned char * pszOptions = NULL;
	unsigned char * pszStringBinding = NULL;
	unsigned long ulCode;

	// local variables
	int				i, j, k, l;
	unsigned long	status;
	unsigned long	curtime;
	float			fbank[AUDIO_BLOCK_SIZE];
	unsigned char	vframe[VIDEO_BLOCK_SIZE];
	float			vtpars[VTRACT_SIZE];
	time_t			tim;

	// initialise random number generator
	tim = time((time_t *)0);
	srand((unsigned)tim);

	// if network address supplied as argument, use that
	if (argc>1) pszNetworkAddress = (unsigned char *)argv[1];

	// compose binding address
	if (RpcStringBindingCompose(pszUuid, pszProtocolSequence, pszNetworkAddress, pszEndpoint, pszOptions, &pszStringBinding)) {
		fprintf(stderr, "Failed to perform RPC string binding composition\n");
		return 1;
	}

	// bind to server
	if (RpcBindingFromStringBinding(pszStringBinding, &klairserver_IfHandle)) {
		fprintf(stderr, "Failed to perform RPC string binding\n");
		return 1;
	}

	// wrap calls in exception handler
	//RpcTryExcept
	//{
		printf("Getting status .. (check that Klair Server is running.\n");
		status = KlairServerGetStatus();
		printf("KlairServerGetStatus() returns %ld\n", status);

		printf("Getting time ...\n");
		curtime = KlairServerGetTime();
		printf("KlairServerGetTime() returns %ld\n", curtime);

		//if (KlairServerGetStatus() & 1) {
		//	// audio is running - get some
		//	printf("Getting audio ...\n");
		//	status = KlairServerGetAudio(curtime - 200, fbank);
		//	printf("KlairServerGetAudio() returns %ld\n", status);
		//	printf("fbank=\n");
		//	for (j = 0; j<10; j++) {
		//		printf("%d:", j);
		//		for (i = 0; i<KS_AUDIO_NUMCHAN; i++) printf("%g,", fbank[j*AUDIO_FRAME_SIZE + i]);
		//		//Bottle&out = outPort.prepare();
		//		//out.clear();
		//		//out.addString("En");
		//		//out.addDouble(fbank[j*AUDIO_FRAME_SIZE + KS_AUDIO_EN]);
		//		//printf("Sending %s\n", out.toString().c_str());
		//		//// send the message
		//		//outPort.write(true);
		//		printf("En=%g,", fbank[j*AUDIO_FRAME_SIZE + KS_AUDIO_EN]);
		//		printf("Fx=%d,", (int)fbank[j*AUDIO_FRAME_SIZE + KS_AUDIO_FX]);
		//		printf("\n");
		//	}
		

		//if (KlairServerGetStatus() & 2) {
		//	// camera is running - get a picture
		//	printf("Getting video ...\n");
		//	status = KlairServerGetVideo(curtime - 200, vframe);
		//	printf("KlairServerGetVideo() returns %ld\n", status);
		//	printf("vframe=\n");
		//	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		//	DWORD count;
		//	// map image to size of console
		//	for (i = 23; i >= 0; i--) {
		//		for (j = 0; j<80; j++) {
		//			int r = 0, g = 0, b = 0;
		//			for (k = 0; k<10; k++)
		//			for (l = 0; l<4; l++) {
		//				b += vframe[3 * 320 * (10 * i + k) + 3 * (4 * j + l)];
		//				g += vframe[3 * 320 * (10 * i + k) + 3 * (4 * j + l) + 1];
		//				r += vframe[3 * 320 * (10 * i + k) + 3 * (4 * j + l) + 2];
		//			}
		//			int col = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		//			if (r >(40 * 127)) col |= BACKGROUND_RED;
		//			if (g >(40 * 127)) col |= BACKGROUND_GREEN;
		//			if (b > (40 * 127)) col |= BACKGROUND_BLUE;
		//			SetConsoleTextAttribute(hStdOut, col);
		//			WriteConsole(hStdOut, " ", 1, &count, NULL);
		//		}
		//	}
		//	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		//	printf("\n");
		//}

while (true) {
	if (KlairServerGetStatus() & 4) {

		std::string expr;

		Bottle* b = inExpressionPort.read(false);
		if (b != NULL) {
			printf("Send an expression\n");

			Value v = b->get(0);
			int i = v.asInt();


			switch (i) {
			case 0:
				printf("Set expression happy\n");
				KlairServerQueueExpress(0, expression_happy);
				break;
			case 1:
				printf("Set expression confused\n");
				KlairServerQueueExpress(0, expression_confused);
				break;
			case 2:
				printf("Set expression bored\n");
				KlairServerQueueExpress(0, expression_bored);
				break;
			case 3:
				printf("Set expression angry\n");
				KlairServerQueueExpress(0, expression_angry);
				break;
			case 4:
				printf("Set expression sad\n");
				KlairServerQueueExpress(0, expression_sad);
				break;
			case 5:
				printf("Set expression sad2\n");
				KlairServerQueueExpress(0, expression_sad2);
				break;
			}
		}




	}



	if (KlairServerGetStatus() & 8) {

		float articulators[VTRACT_SIZE];
		int delay;

		Bottle* b = inVocalTractPort.read(false);
		if (b != NULL) {
			printf("Send a tract gesture\n");
			
			for (int i = 0; i < b->size() - 1; i++) {
				articulators[i] = b->get(i).asDouble();
				printf("ART%d = %f ; ", i, articulators[i]);
			}
			delay = b->get(b->size() - 1).asInt();
			printf("Delay = %d", delay);
			printf("\n");

			memcpy(vtpars, articulators, sizeof(vtpars));

			KlairServerQueueVTract(delay, vtpars);
		}
	}

}
	//}
	//RpcExcept(1)
	//{
	//	ulCode = RpcExceptionCode();
	//	printf("Runtime reported exception 0x%lx = %ld\n", ulCode, ulCode);
	//}
	//RpcEndExcept

	//	// that's all folks
	//	RpcStringFree(&pszStringBinding);
	//RpcBindingFree(&klairserver_IfHandle);
	//return 0;
}

/******************************************************/
/*         MIDL allocate and free                     */
/******************************************************/

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}

