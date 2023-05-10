#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <queue>
#include <tuple>
#include <utility>
#include <map>
using namespace std;

#define decryptiom_mode

class Image {
public:
	int width;
	int height;
	int max_value;
	Image();
};

Image::Image() {
	width = 0;
	height = 0;
	max_value = 0;
}

//Tの第一要素：出現頻度，第二要素：子のノード番号，第三要素：対応する符号語
void initialize_huffman_tree(map<int, int> &dist, vector<tuple<int, vector<int>, string> > &T){
    int i = 0;
    vector<int> temp = {};
    for (auto itr : dist){
        T[i] = make_tuple(dist[itr.first], temp, "");
        i++;
    }
}

//priority_queueの第一要素：出現頻度とそのインデックスのペア，第二要素：内部コンテナの型(格納する要素の型)，第三要素：要素の優先度(今回は値の昇順に指定)
void initialize_priority_queue(map<int, int> &dist, priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > &pque){
    int i = 0;
    for (auto itr : dist){
        pque.push(make_pair(itr.second, i));
        i++;
    }
}

void make_huffman_tree(map<int, int> &dist, vector<tuple<int, vector<int>, string> > &T, priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > &pque){
    for (int i = (int) dist.size(); i < 2 * dist.size() - 1; i++){
        pair<int, int> temp1 = pque.top();
        pque.pop();
        pair<int, int> temp2 = pque.top();
        pque.pop();
        pair<int, int> temp3 = make_pair(temp1.first + temp2.first, i);
        pque.push(temp3);
        vector<int> temp4 = {temp1.second, temp2.second};
        T[i] = make_tuple(temp3.first, temp4, "");
    }
}

//グラフの根元から探索開始して，葉に行くまで深さ優先探索を行う
//vは，根がある位置の要素番号
void huffman_tree_dfs(vector<tuple<int, vector<int>, string> > &T, int v){
	vector<int> temp1 = {};
	if(get<1>(T[v]) != temp1){
		for(int i = 0; i < 2; i++){
			string temp2 = get<2>(T[v]) + (char) (i + '0'); //今いるノードに親のノードを参照し，符号語を子につける．
			int index = get<1>(T[v])[i]; //i = 0の時，出現頻度が小さいほうの子のインデックスを得る．i=1の時，出現頻度が大きいほうの子のインデックスを得る．
			T[index] =  make_tuple(get<0>(T[index]), get<1>(T[index]), temp2);
			huffman_tree_dfs(T, index); //このインデックスからさらに潜る
		}
	}
}


int main(int ac, char *av[]) {
	FILE *fp;
	char tmp[256];
	Image img;
	unsigned int x, y;

	fp = fopen(av[1], "rb");
	if (fp == NULL) {
		cout << "入力ファイルがありません" << endl;
		exit(0);
	}

	fgets(tmp, 256, fp);

	fgets(tmp, 256, fp);
	sscanf(tmp,"%d %d",&img.width, &img.height);
	//vector<vector<unsigned int>> value(img.height, vector<unsigned int>(img.width));

	fgets(tmp, 256, fp);
	sscanf(tmp, "%d", &img.max_value);

    vector<int> S(img.height * img.width);
	for (int y = 0; y < img.height; y++) {
		for (int x = 0; x < img.width; x++) {
            S[y * img.width + x] = (int) fgetc(fp);
		}
	}
	fclose(fp);

	map<int, int> dist;
	for(int i = 0; i < S.size(); i++){
		dist[S[i]]++;
	}
	int N = dist.size();
	//各輝度値に対するノードを作成
	vector<tuple<int, vector<int>, string>> T(2 * N - 1);
	initialize_huffman_tree(dist, T);

	//priority_queueに，各輝度値の出現頻度とそのインデックスを格納
	priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > pque;
    initialize_priority_queue(dist, pque);

	//priority_queue内の出現頻度を足すことでハフマン木を作成する
	make_huffman_tree(dist, T, pque);

	//深さ優先探索で，各ノードに符号を割り当てる
	huffman_tree_dfs(T, 2 * N - 2);

	map<int, string> codeword; //符号語
	int i = 0;
	for(auto itr : dist){
		if(i < N){
			codeword[itr.first] = get<2>(T[i]);
			i++;
		}
	}

	string bits = "";
	for(int i = 0; i < S.size(); i++){
		bits += codeword[S[i]];
	}	

	cout << "before compressing : 8bits/pixel" << endl;
	cout << "after compressing : " << double(bits.size()) / double(img.height * img.width) <<"bits/pixcel" << endl;

#ifdef decryptiom_mode
	string buffer = "";
	vector<int> decoded_image(img.height * img.width);
	int index = 0;
	for(int i = 0; i < bits.size(); i++){
		buffer += bits[i];
		for(auto itr : codeword){
			if(itr.second == buffer){
				decoded_image[index] = itr.first;
				index++;
				buffer = "";
				break;
			}
		}
	}

	if(S == decoded_image) cout << "Decryption successful!" << endl;
	else cout << "Decryption failure" << endl;
#endif

	return 0;
}