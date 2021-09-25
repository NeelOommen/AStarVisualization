#include <iostream>
#include <vector>
#include <iterator>
#include <chrono>
#include <stb/stb_image.h>
#include <SDL.h>
#include <conio.h>

using namespace std;

const int WIDTH = 800;
const int HEIGHT = 800;

enum class cellType {
	start = 0,
	target = -1,
	wall = 1,
	walkable = 2,
	uninitialized = -2,
	visited = 3,
	considered = 4
};


class cell{
private:
	int x, y;
	cellType type;
	int parentI;
	int parentJ;
public:
	double g, h, f;
	
	void setData(int r, int g, int b, int xCoord, int yCoord, int& sources, int& targets) {
		x = xCoord;
		y = yCoord;
		if (r == 255 && g == 255 && b == 255) {
			type = cellType::walkable;
		}
		else if (r == 255 && g != 255 && b != 255) {
			type = cellType::start;
			sources++;
		}
		else if (r != 255 && g == 255 && b != 255) {
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

	void getParents(int &inI, int &inJ) {
		inI = parentI;
		inJ = parentJ;
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
		if (j >= 0 && j < w && ((mat + (i*w + j))->retType()==cellType::walkable || (mat + (i * w + j))->retType() == cellType::considered || (mat + (i * w + j))->retType() == cellType::target) && !((mat + (i * w + j))->retType() == cellType::visited)) {
			flag = true;
		}
	}
	return flag;
}

void findNextCell(vector<pair<int,int>> v,cell* mat, int w, int h, int& nI, int& nJ) {
	//first find min f cost
	double mF = FLT_MAX;
	for (int i = 0; i < v.size();i++) {
		if ((mat + (v[i].first * w + v[i].second))->f <mF) {
			mF = (mat + (v[i].first * w + v[i].second))->f;
		}
	}

	//points with f equal to minimum
	vector<pair<int, int>> minPoints;
	for (int i = 0; i < v.size(); i++) {
		if ((mat + (v[i].first * w + v[i].second))->f == mF) {
			minPoints.push_back(make_pair(v[i].first, v[i].second));
		}
	}

	//min h cost
	int mI, mJ;
	mI = -1;
	mJ = -1;
	if (minPoints.size() == 1) {
		mI = minPoints[0].first;
		mJ = minPoints[0].second;
	}
	else {
		double minH = FLT_MAX;
		for (int i = 0; i < minPoints.size(); i++) {
			if ((mat + (minPoints[i].first * w + minPoints[i].second))->h < minH) {
				minH = (mat + (minPoints[i].first * w + minPoints[i].second))->h;
				mI = minPoints[i].first;
				mJ = minPoints[i].second;
			}
		}
	}

	//set values 
	nI = mI;
	nJ = mJ;
}

void backgroundGrid(SDL_Window* window, SDL_Renderer* renderer, int w, int h, cell* mat) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	int stepSize = WIDTH / w;
	SDL_Rect r;
	r.w = stepSize - 1;
	r.h = stepSize - 1;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (int i = 0; i < h; i++) {
		r.y = i * stepSize;
		for (int j = 0; j < w; j++) {
			r.x = j * stepSize;
			if ((mat + (i * w + j))->retType() == cellType::wall) {
				SDL_SetRenderDrawColor(renderer, 50, 75, 76, 255);
			}
			else
			if ((mat + (i * w + j))->retType() == cellType::target) {
				SDL_SetRenderDrawColor(renderer, 0, 197, 154,255);
			}
			else {
				SDL_SetRenderDrawColor(renderer, 208, 253, 255, 255);
			}
			SDL_RenderFillRect(renderer, &r);
		}
	}
	SDL_RenderPresent(renderer);
}

void aStarSearch(cell* mat, int sI, int sJ, int eI, int eJ, int w, int h, SDL_Window* window, SDL_Renderer* renderer) {

	//Setting initial conditions for the first cell to be chosen
	(mat + (sI * w + sJ))->setFGH(0.0, 0.0, 0.0);
	(mat + (sI * w + sJ))->setParent(sI, sJ);

	bool foundGoal = false;
	int numVisited = 0;

	int cI, cJ;
	cI = sI;
	cJ = sJ;

	int stepSize = WIDTH / w;
	SDL_Rect r;
	r.w = stepSize - 1;
	r.h = stepSize - 1;

	vector<pair<int, int>> consideredPoints;
	consideredPoints.push_back(make_pair(sI,sJ));

	SDL_Event windowEvent;

	//continue here
	//iterate till goal found, or all cells visited
	for (; !foundGoal && numVisited < (w * h);) {
		//set the current cell as visited/closed
		(mat + (cI * w + cJ))->setType(cellType::visited);
		(mat + (cI * w + cJ))->setFGH(FLT_MAX, FLT_MAX, FLT_MAX);
		remove(consideredPoints.begin(), consideredPoints.end(), make_pair(cI, cJ));
		numVisited++;
		//draw visited cell
		SDL_SetRenderDrawColor(renderer, 239, 206, 151, 255);
		r.y = cI * stepSize;
		r.x = cJ * stepSize;
		SDL_RenderFillRect(renderer, &r);


		//check if this is the goal
		if (cI == eI && cJ == eJ) {
			cout << "\nGoal found at" << cI << " " << cJ;
			foundGoal = true;
			break;
		}

		//update all the valid neighbours
		SDL_SetRenderDrawColor(renderer, 147, 248, 179, 255);
		//1st neighbour (up) i-1,j
		if (isValid(cI - 1, cJ, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.0;
			hN = (cI - 1, cJ, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI - 1) * w + cJ))->f == FLT_MAX || (mat + ((cI - 1) * w + cJ))->f > fN) {
				//update the cells data
				(mat + ((cI - 1) * w + cJ))->setFGH(fN, gN, hN);
				(mat + ((cI - 1) * w + cJ))->setParent(cI, cJ);
				(mat + ((cI - 1) * w + cJ))->setType(cellType::considered);
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI - 1, cJ)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI-1,cJ));
				}
				//mark considered rectangle
				r.y = (cI - 1) * stepSize;
				r.x = (cJ) * stepSize;
				SDL_RenderFillRect(renderer, &r);
			}
		}

		//2nd neighbour (top right) i-1,j+1
		if (isValid(cI - 1, cJ + 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.414;
			hN = (cI - 1, cJ + 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI - 1) * w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI - 1) * w + (cJ + 1)))->f > fN) {
				//update the cells data
				(mat + ((cI - 1) * w + (cJ + 1)))->setFGH(fN, gN, hN);
				(mat + ((cI - 1) * w + (cJ + 1)))->setParent(cI, cJ);
				(mat + ((cI - 1) * w + (cJ + 1)))->setType(cellType::considered);
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI - 1, cJ + 1)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI - 1, cJ + 1));
				}
				//mark considered rectangle
				r.y = (cI - 1) * stepSize;
				r.x = (cJ + 1) * stepSize;
				SDL_RenderFillRect(renderer, &r);
			}
		}

		//3rd neighbour (right) i,j+1
		if (isValid(cI, cJ + 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.0;
			hN = (cI, cJ + 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI)*w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI)*w + (cJ + 1)))->f > fN) {
				//update the cells data
				(mat + ((cI)*w + (cJ + 1)))->setFGH(fN, gN, hN);
				(mat + ((cI)*w + (cJ + 1)))->setParent(cI, cJ);
				(mat + ((cI)*w + (cJ + 1)))->setType(cellType::considered);
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI, cJ + 1)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI, cJ + 1));
				}
				//mark considered rectangle
				r.y = (cI) * stepSize;
				r.x = (cJ + 1) * stepSize;
				SDL_RenderFillRect(renderer, &r);
			}
		}

		//4th neighbour (bottom right) i+1,j+1
		if (isValid(cI + 1, cJ + 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.414;
			hN = (cI + 1, cJ + 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI + 1) * w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI + 1) * w + (cJ + 1)))->f > fN) {
				//update the cells data
				(mat + ((cI + 1) * w + (cJ + 1)))->setFGH(fN, gN, hN);
				(mat + ((cI + 1) * w + (cJ + 1)))->setParent(cI, cJ);
				(mat + ((cI + 1) * w + (cJ + 1)))->setType(cellType::considered);
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI + 1, cJ + 1)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI + 1, cJ + 1));
				}
				//mark considered rectangle
				r.y = (cI + 1) * stepSize;
				r.x = (cJ + 1) * stepSize;
				SDL_RenderFillRect(renderer, &r);
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
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI + 1, cJ)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI + 1, cJ));
				}
				//mark considered rectangle
				r.y = (cI + 1) * stepSize;
				r.x = (cJ) * stepSize;
				SDL_RenderFillRect(renderer, &r);
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
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI + 1, cJ - 1)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI + 1, cJ - 1));
				}
				//mark considered rectangle
				r.y = (cI + 1) * stepSize;
				r.x = (cJ - 1) * stepSize;
				SDL_RenderFillRect(renderer, &r);
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
			if ((mat + ((cI)*w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI)*w + (cJ - 1)))->f > fN) {
				//update the cells data
				(mat + ((cI)*w + (cJ - 1)))->setFGH(fN, gN, hN);
				(mat + ((cI)*w + (cJ - 1)))->setParent(cI, cJ);
				(mat + ((cI)*w + (cJ - 1)))->setType(cellType::considered);
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI, cJ - 1)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI, cJ - 1));
				}
				//mark considered rectangle
				r.y = (cI) * stepSize;
				r.x = (cJ - 1) * stepSize;
				SDL_RenderFillRect(renderer, &r);
			}
		}

		//8th neighbour (top left) i-1,j-1
		if (isValid(cI - 1, cJ - 1, w, h, mat)) {
			//new f,g,h costs
			double fN, hN, gN;
			gN = (mat + (cI * w + cJ))->g + 1.414;
			hN = (cI - 1, cJ - 1, eI, eJ);
			fN = gN + hN;
			//check if cell is uninitialized or new path is more efficient than the old one
			if ((mat + ((cI - 1) * w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI - 1) * w + (cJ - 1)))->f > fN) {
				//update the cells data
				(mat + ((cI - 1) * w + (cJ - 1)))->setFGH(fN, gN, hN);
				(mat + ((cI - 1) * w + (cJ - 1)))->setParent(cI, cJ);
				(mat + ((cI - 1) * w + (cJ - 1)))->setType(cellType::considered);
				if (!(find(consideredPoints.begin(), consideredPoints.end(), make_pair(cI - 1, cJ - 1)) != consideredPoints.end())) {
					consideredPoints.push_back(make_pair(cI - 1, cJ - 1));
				}
				//mark considered rectangle
				r.y = (cI - 1) * stepSize;
				r.x = (cJ - 1) * stepSize;
				SDL_RenderFillRect(renderer, &r);
			}
		}
		SDL_RenderPresent(renderer);

		//pick the next node to visit
		//the node visited next will be the one with the least f cost
		//if cells have the same f cost, then pick the one with the lower h cost
		//if both the f cost and h cost are the same, pick any one of them arbitrarily
		int nI, nJ;
		findNextCell(consideredPoints, mat, w,h,nI,nJ);
		//change to the chosen cell
		cI = nI;
		cJ = nJ;
		if (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) {
				break;
			}
		}

	}
	if (!foundGoal) {
		cout << "\nPath not found";
	}
}



int main(int argc, char* args[]) {
	//initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		//SDL failed to initialize
		cout << "\nSDL Initialization Error: " << SDL_GetError() << endl;
		return 0;
	}

	//Create the SDL window
	SDL_Window* window = SDL_CreateWindow("Pathfinder", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

	//Verify window creation
	if (window == NULL) {
		cout << "\nWindow creation has failed: "<<SDL_GetError()<<endl;
		return 0;
	}

	//sdl renderer
	SDL_Renderer* renderer = NULL;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	auto beginTime = std::chrono::high_resolution_clock::now();
	int width, height, numChannels;
	unsigned char* imgData = stbi_load("Maps/sanity.png", &width, &height, &numChannels, 0);

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
			//cout << i<<","<<j<<":"<<(int)pixelLoc[0] << " "<< (int)pixelLoc[1] << " "<<(int)pixelLoc[2] << " "<<(int)pixelLoc[3]<<endl;
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
	
	backgroundGrid(window, renderer, width, height, mat);

	//actual search
	aStarSearch(mat, startI,startJ, endI, endJ,width,height,window,renderer);

	//debug for the map
	/*cout << "\n";
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			int c = (int)(mat + (i * width + j))->retType();
			if (c == (int)cellType::visited) {
				cout << "X";
			}
			else
			if (c == (int)cellType::wall) {
					cout << "|";
			}
			else {
				cout << ".";
			}
		}
		cout << endl;
	}*/

	//back track actual found path
	int bI, bJ;
	bI = endI;
	bJ = endJ;
	int stepSize = WIDTH/width;
	SDL_Rect bR;
	bR.w = stepSize - 1;
	bR.h = stepSize - 1;
	SDL_SetRenderDrawColor(renderer, 0, 173, 190, 255);
	for (int i=0; width*height;i++) {
		int l, k;
		l = bI;
		k = bJ;
		bR.y = bI * stepSize;
		bR.x= bJ * stepSize;
		SDL_RenderFillRect(renderer, &bR);
		SDL_RenderPresent(renderer);
		(mat + (bI * width + bJ))->getParents(bI,bJ);
		if (bI == l && bJ == k) {
			break;
		}
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - beginTime);
	cout << "\nTotal execution time: " << elapsed.count() * 1e-9;
	_getch();
	//cleanup of data and stb 
	cleanup(imgData,mat);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}