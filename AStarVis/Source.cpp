#include <iostream>
#include <stb/stb_image.h>

using namespace std;

enum class cellType {
	start = 0,
	target = -1,
	wall = 1,
	walkable = 2
};

class cell {
private:
	int x, y;
	cellType type;
public:
	double g, h, f;
	void setData(int r, int g, int b, int xCoord, int yCoord, int &sources, int &targets) {
		x = xCoord;
		y = yCoord;
		if (r == 255 && g == 255 && b == 255) {
			type = cellType::walkable;
		}
		else if (r == 255 && g != 255 && b != 255) {
			type = cellType::start;
			sources++;
		}
		else if ( r != 255 && g == 255 && b != 255) {
			type = cellType::target;
			targets++;
		}
		else {
			type = cellType::wall;
		}
	}
	cellType retType() { return type; }
	int retX() { return x; }
	int retY() { return y; }
};

double hValue(int i,int j,int desti,int destj) {
	return((double)sqrt(((i-desti)*(i-desti))-((j-destj)*(j-destj))));
}

void cleanup(unsigned char* img, cell* cellData) {
	delete cellData;
	stbi_image_free(img);
}

int main() {
	int width, height, numChannels;
	unsigned char* imgData = stbi_load("Maps/testMap16.png", &width, &height, &numChannels, 0);

	cout << "Size of the image: " << width << "x" << height<<endl;

	//load data into matrix of cells
	cell* mat = new cell[width * height];
	unsigned bytePerPixel = numChannels;
	int targets = 0;
	int sources = 0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			unsigned char* pixelLoc = imgData + (i * width + j) * bytePerPixel;
			cout << i<<","<<j<<":"<<(int)pixelLoc[0] << " "<< (int)pixelLoc[1] << " "<<(int)pixelLoc[2] << " "<<(int)pixelLoc[3]<<endl;
			(mat + (i * width + j))->setData(pixelLoc[0],pixelLoc[1],pixelLoc[2],i,j,sources,targets);
		}
	}

	if (sources != 1 || targets != 1) {
		cout << "Error: Invalid map loaded.\nExiting Execution.";
		cleanup(imgData,mat);
		exit(0);
	}

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			cout << (int)(mat + (i*width + j))->retType()<<" ";
		}
		cout << endl;
	}

	cleanup(imgData,mat);
	return 0;
}