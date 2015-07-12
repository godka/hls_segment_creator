#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
typedef struct resolution{
	int width;
	int height;
	int rate;
	int maxrate;
} Tresolution;

Tresolution rlist [] = {
	{ 1920, 1080, 1500, 2000 },
	{ 1280, 720, 800, 1000 },
	{ 640, 360, 400, 500 },
	{ 320, 180, 200, 250 }
};
int GetFileNameWithoutExtension(char* filename){
	int i;
	int tmp = strlen(filename);
	for (i = tmp - 1; i >= 0; i--){
		if (filename[i] == '.'){
			return i;
		}
	}
	return 0;
}
int GetFileName(char* filename){
	int i;
	int tmp = strlen(filename);
	for (i = tmp - 1; i >= 0; i--){
		if (filename[i] == '\\' || filename[i] == '/'){
#ifdef WIN32
			return i + 1;
#else
			return i;
#endif
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	char* filename, *strformat;;
	char tmp[1024] = { 0 }, realfilename[255] = { 0 },pfilename[255] = { 0 };
	int realfileindex, pfileindex,i;
	FILE* istream; 
#ifdef WIN32
	strformat = "start ffmpeg -y -i %s -pix_fmt yuv420p -vcodec libx264 -acodec aac -strict -2 -r 25 -profile:v baseline  -b:v %dk -maxrate %dk -force_key_frames 50 -s %dx%d -map 0 -flags -global_header -f segment -segment_list %s_%d_%d.m3u8 -segment_time 10 -segment_format mpeg_ts -segment_list_type m3u8 %s_%d_%d_%%05d.ts";
#else
	strformat = "ffmpeg -y -i %s -pix_fmt yuv420p -vcodec libx264 -acodec aac -strict -2 -r 25 -profile:v baseline  -b:v %dk -maxrate %dk -force_key_frames 50 -s %dx%d -map 0 -flags -global_header -f segment -segment_list %s_%d_%d.m3u8 -segment_time 10 -segment_format mpeg_ts -segment_list_type m3u8 %s_%d_%d_%%05d.ts &";
#endif
	if (argc > 1){
		filename = argv[1];
		realfileindex = GetFileNameWithoutExtension(filename);
		pfileindex = GetFileName(filename);
		if (pfileindex - realfileindex > 0){
			realfileindex = strlen(filename);
		}
		memcpy(realfilename, filename + pfileindex, realfileindex - pfileindex);
		memcpy(pfilename, filename + pfileindex, strlen(filename) - pfileindex);
		puts(pfilename);
		sprintf(tmp, "%s.m3u8", realfilename);
		istream = fopen(tmp, "w");
		fprintf(istream, "#EXTM3U\n");
		for (i = 0; i < sizeof(rlist) / sizeof(Tresolution); i++){
			sprintf(tmp, strformat, 
				pfilename,rlist[i].rate, rlist[i].maxrate,
				rlist[i].width, rlist[i].height,
				realfilename, rlist[i].width, rlist[i].height,
				realfilename, rlist[i].width, rlist[i].height);
			fprintf(istream, "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=%d000\n", rlist[i].maxrate);
			fprintf(istream,"%s_%d_%d.m3u8\n", realfilename, rlist[i].width, rlist[i].height);
				//execl(tmp,"ffmpeg",NULL);
			system(tmp);
		}
		fprintf(istream, "#EXT-X-ENDLIST");
		fclose(istream);
		puts("done");
	}
	else{
		puts("usage:hls_segment_creator filename");
	}
#ifdef WIN32
	getchar();
#endif
	return 0;
}

