#include <iostream>
#include <stb/stb_image.h>

using namespace std;

enum class cellType {
	start = 0,
	target = -1,
	wall = 1,
	walkable = 2,
	uninitialized = -2,
	visited = 3,
	considered = 4
};

class cell {
private:
	int x, y;
	cellType type;
	int parentI;;
	int parentJ;
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
	void setFGH(double _f, double _g, double _h) {
		f = _f;
		g = _g;
		h = _h;
	}
	void setParent(int _i, int _j) {
		parentI = _i;
		parentJ = _j;
	}

	void setType(enum class cellType t) {
		type = t;
	}

	cell() {
		type = cellType::uninitialized;
		x = -1;
		y = -1;
		f = FLT_MAX;
		g = FLT_MAX;
		h = FLT_MAX;
		parentI = -1;
		parentJ = -1;
	}
};

double hValue(int i,int j,int desti,int destj) {
	return((double)sqrt(((i-desti)*(i-desti))-((j-destj)*(j-destj))));
}

void cleanup(unsigned char* img, cell* cellData) {
	delete cellData;
	stbi_image_free(img);
}

bool isValid(int i, int j, int w, int h, cell* mat) {
	bool flag = false;
	if (i >= 0 && i < h) {
		if (j >= 0 && j < w && (mat + (i*w + j))->retType()!=cellType::wall) {
			flag = true;
		}
	}
	return flag;
}

void aStarSearch(cell* mat, int sI, int sJ, int eI, int eJ,int w, int h) {

	//Setting initial conditions for the first cell to be chosen
	(mat + (sI * w + sJ))->setFGH(0.0,0.0,0.0);
	(mat + (sI * w + sJ))->setParent(sI,sJ);

	bool foundGoal = false;
	bool* visited = new bool[w*h];
	int numVisited = 0;

	int cI, cJ;
	cI = sI;
	cJ = sJ;

	//continue here
	//iterate till goal found, or all cells visited
	for (;!foundGoal && numVisited<w*h;) {
		//set the current cell as visited/closed
		(mat + (cI * w + cJ))->setType(cellType::visited);
		*(visited + (cI * w + cJ)) = true;
		numVisited++;

		//check if this is the goal
		if (cI == eI && cJ == eJ) {
			cout << "\nGoal found at" << cI << " " << cJ;
			foundGoal = true;
			break;
		}

		//update all the valid neighbours
		//1st neighbour (up) i-1,j
		if (isValid(cI - 1, cJ, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.0;
			hN = (cI-1,cJ,eI,eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI-1) * w + cJ))->f == FLT_MAX || (mat + ((cI-1) * w + cJ))->f > fN) {
				//update the cells data
				(mat + ((cI - 1) * w + cJ))->setFGH(fN,gN,hN);
				(mat + ((cI - 1) * w + cJ))->setParent(cI,cJ);
				(mat + ((cI - 1) * w + cJ))->setType(cellType::considered);
			}
		}

		//2nd neighbour (top right) i-1,j+1
		if (isValid(cI - 1, cJ + 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.414;
			hN = (cI - 1, cJ+1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI - 1) * w + (cJ+1)))->f == FLT_MAX || (mat + ((cI - 1) * w + (cJ+1)))->f > fN) {
				//update the cells data
				(mat + ((cI - 1) * w + (cJ + 1)))->setFGH(fN, gN, hN);
				(mat + ((cI - 1) * w + (cJ + 1)))->setParent(cI, cJ);
				(mat + ((cI - 1) * w + (cJ + 1)))->setType(cellType::considered);
			}
		}

		//3rd neighbour (right) i,j+1
		if (isValid(cI, cJ + 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.0;
			hN = (cI , cJ + 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI) * w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI) * w + (cJ + 1)))->f > fN) {
				//update the cells data
				(mat + ((cI) * w + (cJ + 1)))->setFGH(fN, gN, hN);
				(mat + ((cI) * w + (cJ + 1)))->setParent(cI, cJ);
				(mat + ((cI) * w + (cJ + 1)))->setType(cellType::considered);
			}
		}

		//4th neighbour (bottom right) i+1,j+1
		if (isValid(cI + 1, cJ + 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.414;
			hN = (cI+1, cJ + 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI + 1)*w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI + 1)*w + (cJ + 1)))->f > fN) {
				//update the cells data
				(mat + ((cI + 1)*w + (cJ + 1)))->setFGH(fN, gN, hN);
				(mat + ((cI + 1)*w + (cJ + 1)))->setParent(cI, cJ);
				(mat + ((cI + 1)*w + (cJ + 1)))->setType(cellType::considered);
			}
		}

		//5th neighbour (bottom) i+1,j
		if (isValid(cI + 1, cJ, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.0;
			hN = (cI + 1, cJ, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI + 1) * w + (cJ)))->f == FLT_MAX || (mat + ((cI + 1) * w + (cJ)))->f > fN) {
				//update the cells data
				(mat + ((cI + 1) * w + (cJ)))->setFGH(fN, gN, hN);
				(mat + ((cI + 1) * w + (cJ)))->setParent(cI, cJ);
				(mat + ((cI + 1) * w + (cJ)))->setType(cellType::considered);
			}
		}

		//6th neighbour (bottom left) i+1,j-1
		if (isValid(cI + 1, cJ - 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.414;
			hN = (cI + 1, cJ - 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI + 1) * w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI + 1) * w + (cJ - 1)))->f > fN) {
				//update the cells data
				(mat + ((cI + 1) * w + (cJ - 1)))->setFGH(fN, gN, hN);
				(mat + ((cI + 1) * w + (cJ - 1)))->setParent(cI, cJ);
				(mat + ((cI + 1) * w + (cJ - 1)))->setType(cellType::considered);
			}
		}

		//7th neighbour (left) i,j-1
		if (isValid(cI, cJ - 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.0;
			hN = (cI, cJ - 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI) * w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI) * w + (cJ - 1)))->f > fN) {
				//update the cells data
				(mat + ((cI) * w + (cJ - 1)))->setFGH(fN, gN, hN);
				(mat + ((cI) * w + (cJ - 1)))->setParent(cI, cJ);
				(mat + ((cI) * w + (cJ - 1)))->setType(cellType::considered);
			}
		}

		//8th neighbour (top left) i-1,j-1
		if (isValid(cI-1, cJ - 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.414;
			hN = (cI-1, cJ - 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI - 1)*w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI - 1)*w + (cJ - 1)))->f > fN) {
				//update the cells data
				(mat + ((cI - 1) * w + (cJ - 1)))->setFGH(fN, gN, hN);
				(mat + ((cI - 1) * w + (cJ - 1)))->setParent(cI, cJ);
				(mat + ((cI - 1) * w + (cJ - 1)))->setType(cellType::considered);
			}
		}

		//pick the next node to visit
		//the node visited next will be the one with the least f cost
		//if cells have the same f cost, then pick the one with the lower h cost
		//if both the f cost and h cost are the same, pick any one of them arbitrarily

	}
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
	int startI = -1;
	int startJ = -1;
	int endI = -1;
	int endJ = -1;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			unsigned char* pixelLoc = imgData + (i * width + j) * bytePerPixel;
			cout << i<<","<<j<<":"<<(int)pixelLoc[0] << " "<< (int)pixelLoc[1] << " "<<(int)pixelLoc[2] << " "<<(int)pixelLoc[3]<<endl;
			(mat + (i * width + j))->setData(pixelLoc[0],pixelLoc[1],pixelLoc[2],i,j,sources,targets);
			if (pixelLoc[0] != 255 && pixelLoc[1] == 255 && pixelLoc[2] != 255) {
				endI = i;
				endJ = j;
			}
			else if (pixelLoc[0] == 255 && pixelLoc[1] != 255 && pixelLoc[2] != 255) {
				startI = i;
				startJ = j;
			}
		}
	}

	//if loading the map fails
	if (sources != 1 || targets != 1) {
		cout << "Error: Invalid map loaded.\nExiting Execution.";
		cleanup(imgData,mat);
		exit(0);
	}

	//actual search
	aStarSearch(mat, startI,startJ, endI, endJ,width,height);

	//debug for the map
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			cout << (int)(mat + (i*width + j))->retType()<<" ";
		}
		cout << endl;
	}

	//cleanup of data and stb 
	cleanup(imgData,mat);
	return 0;
}