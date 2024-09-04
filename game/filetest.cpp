#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <GL/gl.h>

// C++17以降で使用可能なfilesystem
namespace fs = std::filesystem;

struct Objdata {
    std::vector<GLfloat> box_min;
    std::vector<GLfloat> box_max;
    std::vector<GLfloat> vertex;
    std::vector<GLfloat> flat;
    std::vector<GLfloat> sortedVertex;
    std::vector<GLfloat> sortedFlat;
    std::vector<GLfloat> UV;
    std::vector<int> index;
    std::string filename;       // ファイル名を格納
};

// 関数プロトタイプ
void loadObjData(const std::string& filename, Objdata& obj);

int main() {
    std::string objDirectoryPath = "../obj"; // objディレクトリのパス
    std::vector<Objdata> objDataArray;

    // すべてのファイルをベクターに格納
    std::vector<fs::path> filePaths;
    for (const auto& entry : fs::directory_iterator(objDirectoryPath)) {
        if (entry.is_regular_file()) {
            filePaths.push_back(entry.path());
        }
    }

    // ファイル名でソート
    std::sort(filePaths.begin(), filePaths.end());

    // ソートされたファイルを処理
    for (const auto& path : filePaths) {
        Objdata obj;
        loadObjData(path.string(), obj);
        objDataArray.push_back(obj);

        // ソートされたファイル名のみを出力する
        std::cout << "Loaded file: " << obj.filename << std::endl;
    }
    return 0;
}

void loadObjData(const std::string& filename, Objdata& obj)
{
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    // ファイル名を構造体に保存
    obj.filename = fs::path(filename).filename().string();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream linestream(line);
        std::string prefix;
        linestream >> prefix;

        if (prefix == "s,") {
            GLfloat x, y, z;
            char comma;
            linestream >> comma >> x >> comma >> y >> comma >> z;
            obj.vertex.push_back(x);
            obj.vertex.push_back(y);
            obj.vertex.push_back(z);
        }
        else if (prefix == "b,") {
            GLfloat x, y, z;
            char comma;
            linestream >> comma >> x >> comma >> y >> comma >> z;
            obj.vertex.push_back(x);
            obj.vertex.push_back(y);
            obj.vertex.push_back(z);
        }
        else if (prefix == "v,") {
            GLfloat x, y, z;
            char comma;
            linestream >> comma >> x >> comma >> y >> comma >> z;
            obj.vertex.push_back(x);
            obj.vertex.push_back(y);
            obj.vertex.push_back(z);
        }
        else if (prefix == "n,") {
            GLfloat nx, ny, nz;
            char comma;
            linestream >> comma >> nx >> comma >> ny >> comma >> nz;
            obj.flat.push_back(nx);
            obj.flat.push_back(ny);
            obj.flat.push_back(nz);
        }
        else if (prefix == "i,") {
            int idx1, idx2, idx3;
            char comma;
            linestream >> comma >> idx1 >> comma >> idx2 >> comma >> idx3;
            obj.index.push_back(idx1);
            obj.index.push_back(idx2);
            obj.index.push_back(idx3);
        }
        else if (prefix == "u,") {
            GLfloat u, v;
            char comma;
            linestream >> comma >> u >> comma >> v;
            obj.UV.push_back(u);
            obj.UV.push_back(v);
        }
    }

    obj.sortedVertex.resize(obj.index.size() * 3);
    obj.sortedFlat.resize(obj.index.size() * 3);
    for (size_t i = 0; i < obj.index.size(); i++) {
        int idx = obj.index[i];
        obj.sortedVertex[i * 3] = obj.vertex[idx * 3];
        obj.sortedVertex[i * 3 + 1] = obj.vertex[idx * 3 + 1];
        obj.sortedVertex[i * 3 + 2] = obj.vertex[idx * 3 + 2];
        obj.sortedFlat[i * 3] = obj.flat[idx * 3];
        obj.sortedFlat[i * 3 + 1] = obj.flat[idx * 3 + 1];
        obj.sortedFlat[i * 3 + 2] = obj.flat[idx * 3 + 2];
    }
}
