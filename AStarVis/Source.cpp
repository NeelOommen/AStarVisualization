#include <iostream>
#include <chrono>
#include <stb/stb_image.h>
#include <SDL.h>
#include <conio.h>
#include <set>

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
	double g;
	double h;
	double f;
	
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
	void setFGH(double fI, double gI, double hI) {
		f = fI;
		g = gI;
		h = hI;
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
		parentI = INT_MAX;
		parentJ = INT_MAX;
	}

	void getParents(int &i, int &j) {
		i = parentI;
		j = parentJ;
	}

};



double hValue(int i,int j,int desti,int destj) {
	return((double)sqrt(((i-desti)*(i-desti))+((j-destj)*(j-destj))));
}

void cleanup(unsigned char* img, cell* cellData) {
	delete cellData;
	stbi_image_free(img);
}

bool isValid(int i, int j, int w, int h, cell* mat) {
	bool flag = false;
	if (i >= 0 && i < h) {
		if (j >= 0 && j < w) {
			if (((mat + (i * w + j))->retType() == cellType::walkable || (mat + (i * w + j))->retType() == cellType::considered || (mat + (i * w + j))->retType() == cellType::target) && (mat + (i * w + j))->retType() != cellType::visited && (mat + (i * w + j))->retType() != cellType::wall) {
				flag = true;
			}
		}
	}
	return flag;
}
//((mat + (i*w + j))->retType()==cellType::walkable || (mat + (i * w + j))->retType() == cellType::considered || (mat + (i * w + j))->retType() == cellType::target) && !((mat + (i * w + j))->retType() == cellType::visited

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
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			}
			else
			if ((mat + (i * w + j))->retType() == cellType::target) {
				SDL_SetRenderDrawColor(renderer, 0, 255, 0,255);
			}
			else {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			}
			SDL_RenderFillRect(renderer, &r);
		}
	}
	SDL_RenderPresent(renderer);
}

void checkNeighbour(set<pair<double, pair<int,int>>> &cons, int i, int j, int offsetX, int offsetY, int endI, int endJ, int width, int height, cell* mat, SDL_Renderer* renderer, SDL_Rect rect, int stepSize) {
	//check validity of the cell
	if (isValid(i + offsetX, j + offsetY, width, height, mat)) {
		//calculate new costs for this cell
		double fN, gN, hN;
		gN = (mat + (i * width + j))->g + (offsetX != 0 && offsetY != 0) ? 1.414 : 1.0;
		hN = hValue(i + offsetX, j + offsetY, endI, endJ);
		fN = gN + hN;

		//check if cell in uninitialized or if the new path is shorter than the previous one to reach this cell
		if ((mat + ((i + offsetX) * width + (j + offsetY)))->f == FLT_MAX || (mat + ((i + offsetX) * width + (j + offsetY)))->f > fN) {
			//update cell data
			(mat + ((i + offsetX) * width + (j + offsetY)))->setFGH(fN, gN, hN);
			(mat + ((i + offsetX) * width + (j + offsetY)))->setParent(i, j);
			(mat + ((i + offsetX) * width + (j + offsetY)))->setType(cellType::considered);
			cons.insert(make_pair(fN, make_pair(i + offsetX, j + offsetY)));

			//mark considered rectangle
			rect.y = (i + offsetX) * stepSize;
			rect.x = (j + offsetY) * stepSize;
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

void aStarSearch(cell* mat, int sI, int sJ, int eI, int eJ, int w, int h, SDL_Window* window, SDL_Renderer* renderer) {

	//Setting initial conditions for the first cell to be chosen
	(mat + (sI * w + sJ))->setFGH(0.0, 0.0, 0.0);
	(mat + (sI * w + sJ))->setParent(-1, -1);

	set<pair<double,pair<int,int>>> cons;

	bool foundGoal = false;
	int numVisited = 0;

	int cI, cJ;
	cI = sI;
	cJ = sJ;

	int stepSize = WIDTH / w;
	SDL_Rect r;
	r.w = stepSize - 1;
	r.h = stepSize - 1;

	cons.insert(make_pair(0.0, make_pair(sI, sJ)));

	SDL_Event windowEvent;

	//continue here
	//iterate till goal found, or all cells visited
	for (; !foundGoal && numVisited < (w * h) && cons.size() > 0;) {
		//pick the point
		pair<double, pair<int, int>> p = *cons.begin();
		cI = p.second.first;
		cJ = p.second.second;

		cons.erase(cons.begin());
		//set the current cell as visited/closed
		if (!(cI >= 0 && cI < h) || !(cJ >= 0 && cJ < w)) {
			cout << "\nInvalid Index "<<cI<<" "<<cJ;
			break;
		}
		(mat + (cI * w + cJ))->setType(cellType::visited);

		numVisited++;
		//draw visited cell
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
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
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//1st neighbour (up) i-1,j
		//if (isValid(cI - 1, cJ, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.0;
		//	hN = hValue(cI - 1, cJ, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI - 1) * w + cJ))->f == FLT_MAX || (mat + ((cI - 1) * w + cJ))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI - 1) * w + cJ))->setFGH(fN, gN, hN);
		//		(mat + ((cI - 1) * w + cJ))->setParent(cI, cJ);
		//		(mat + ((cI - 1) * w + cJ))->setType(cellType::considered);
		//		cons.insert(make_pair(fN,make_pair(cI-1,cJ)));
		//		
		//		//mark considered rectangle
		//		r.y = (cI - 1) * stepSize;
		//		r.x = (cJ) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ, -1, 0, eI, eJ,  w, h, mat, renderer, r, stepSize);

		//2nd neighbour (top right) i-1,j+1
		//if (isValid(cI - 1, cJ + 1, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.414;
		//	hN = hValue(cI - 1, cJ + 1, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI - 1) * w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI - 1) * w + (cJ + 1)))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI - 1) * w + (cJ + 1)))->setFGH(fN, gN, hN);
		//		(mat + ((cI - 1) * w + (cJ + 1)))->setParent(cI, cJ);
		//		(mat + ((cI - 1) * w + (cJ + 1)))->setType(cellType::considered);
		//		cons.insert(make_pair(fN, make_pair(cI - 1, cJ+1)));
		//		
		//		//mark considered rectangle
		//		r.y = (cI - 1) * stepSize;
		//		r.x = (cJ + 1) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ, -1, 1, eI, eJ, w, h, mat, renderer, r, stepSize);

		//3rd neighbour (right) i,j+1
		//if (isValid(cI, cJ + 1, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.0;
		//	hN = hValue(cI, cJ + 1, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI)*w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI)*w + (cJ + 1)))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI)*w + (cJ + 1)))->setFGH(fN, gN, hN);
		//		(mat + ((cI)*w + (cJ + 1)))->setParent(cI, cJ);
		//		(mat + ((cI)*w + (cJ + 1)))->setType(cellType::considered);
		//		cons.insert(make_pair(fN, make_pair(cI, cJ+1)));
		//		
		//		//mark considered rectangle
		//		r.y = (cI) * stepSize;
		//		r.x = (cJ + 1) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ, 0, 1, eI, eJ, w, h, mat, renderer, r, stepSize);

		//4th neighbour (bottom right) i+1,j+1
		//if (isValid(cI + 1, cJ + 1, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.414;
		//	hN = hValue(cI + 1, cJ + 1, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI + 1) * w + (cJ + 1)))->f == FLT_MAX || (mat + ((cI + 1) * w + (cJ + 1)))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI + 1) * w + (cJ + 1)))->setFGH(fN, gN, hN);
		//		(mat + ((cI + 1) * w + (cJ + 1)))->setParent(cI, cJ);
		//		(mat + ((cI + 1) * w + (cJ + 1)))->setType(cellType::considered);
		//		cons.insert(make_pair(fN, make_pair(cI + 1, cJ + 1)));
		//		
		//		//mark considered rectangle
		//		r.y = (cI + 1) * stepSize;
		//		r.x = (cJ + 1) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ, 1, 1, eI, eJ, w, h, mat, renderer, r, stepSize);

		//5th neighbour (bottom) i+1,j
		//if (isValid(cI + 1, cJ, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.0;
		//	hN = hValue(cI + 1, cJ, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI + 1) * w + (cJ)))->f == FLT_MAX || (mat + ((cI + 1) * w + (cJ)))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI + 1) * w + (cJ)))->setFGH(fN, gN, hN);
		//		(mat + ((cI + 1) * w + (cJ)))->setParent(cI, cJ);
		//		(mat + ((cI + 1) * w + (cJ)))->setType(cellType::considered);
		//		cons.insert(make_pair(fN, make_pair(cI + 1, cJ)));
		//		
		//		//mark considered rectangle
		//		r.y = (cI + 1) * stepSize;
		//		r.x = (cJ) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ, 1, 0, eI, eJ, w, h, mat, renderer, r, stepSize);

		//6th neighbour (bottom left) i+1,j-1
		//if (isValid(cI + 1, cJ - 1, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.414;
		//	hN = hValue(cI + 1, cJ - 1, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI + 1) * w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI + 1) * w + (cJ - 1)))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI + 1) * w + (cJ - 1)))->setFGH(fN, gN, hN);
		//		(mat + ((cI + 1) * w + (cJ - 1)))->setParent(cI, cJ);
		//		(mat + ((cI + 1) * w + (cJ - 1)))->setType(cellType::considered);
		//		cons.insert(make_pair(fN, make_pair(cI + 1, cJ - 1)));
		//		
		//		//mark considered rectangle
		//		r.y = (cI + 1) * stepSize;
		//		r.x = (cJ - 1) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ, 1, -1, eI, eJ, w, h, mat, renderer, r, stepSize);

		//7th neighbour (left) i,j-1
		//if (isValid(cI, cJ - 1, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.0;
		//	hN = hValue(cI, cJ - 1, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI)*w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI)*w + (cJ - 1)))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI)*w + (cJ - 1)))->setFGH(fN, gN, hN);
		//		(mat + ((cI)*w + (cJ - 1)))->setParent(cI, cJ);
		//		(mat + ((cI)*w + (cJ - 1)))->setType(cellType::considered);
		//		cons.insert(make_pair(fN, make_pair(cI, cJ - 1)));
		//		
		//		//mark considered rectangle
		//		r.y = (cI) * stepSize;
		//		r.x = (cJ - 1) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ,0, -1, eI, eJ, w, h, mat, renderer, r, stepSize);

		//8th neighbour (top left) i-1,j-1
		//if (isValid(cI - 1, cJ - 1, w, h, mat)) {
		//	//new f,g,h costs
		//	double fN, hN, gN;
		//	gN = (mat + (cI * w + cJ))->g + 1.414;
		//	hN = hValue(cI - 1, cJ - 1, eI, eJ);
		//	fN = hN + gN;
		//	//check if cell is uninitialized or new path is more efficient than the old one
		//	if ((mat + ((cI - 1) * w + (cJ - 1)))->f == FLT_MAX || (mat + ((cI - 1) * w + (cJ - 1)))->f > fN) {
		//		//update the cells data
		//		(mat + ((cI - 1) * w + (cJ - 1)))->setFGH(fN, gN, hN);
		//		(mat + ((cI - 1) * w + (cJ - 1)))->setParent(cI, cJ);
		//		(mat + ((cI - 1) * w + (cJ - 1)))->setType(cellType::considered);
		//		cons.insert(make_pair(fN, make_pair(cI - 1, cJ - 1)));

		//		//mark considered rectangle
		//		r.y = (cI - 1) * stepSize;
		//		r.x = (cJ - 1) * stepSize;
		//		SDL_RenderFillRect(renderer, &r);
		//	}
		//}

		checkNeighbour(cons, cI, cJ, -1, -1, eI, eJ, w, h, mat, renderer, r, stepSize);


		SDL_RenderPresent(renderer);

		//pick the next node to visit
		//the node visited next will be the one with the least f cost
		//if cells have the same f cost, then pick the one with the lower h cost
		//if both the f cost and h cost are the same, pick any one of them arbitrarily
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
	SDL_Window* window = SDL_CreateWindow("20BCE2000, Neel Ittyerah Oommen", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

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
	unsigned char* imgData = stbi_load("Maps/finalMaze.png", &width, &height, &numChannels, 0);

	cout << "Size of the image: " << width << "x" << height<<endl;

	//load data into matrix of cell
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
	else {
		cout << "\nStart: " << startI << " " << startJ;
		cout << "\nEnd: " << endI << " " << endJ;
	}
	
	backgroundGrid(window, renderer, width, height, mat);

	//actual search
	aStarSearch(mat, startI,startJ, endI, endJ,width,height,window,renderer);

	//back track actual found path
	int bI, bJ;
	bI = endI;
	bJ = endJ;
	int stepSize = WIDTH/width;
	SDL_Rect bR;
	bR.w = stepSize - 1;
	bR.h = stepSize - 1;
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
	for (int i=0; width*height;i++) {
		bR.y = bI * stepSize;
		bR.x= bJ * stepSize;
		SDL_RenderFillRect(renderer, &bR);
		SDL_RenderPresent(renderer);
		(mat + (bI * width + bJ))->getParents(bI,bJ);
		if (bI == -1 && bJ == -1) {
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