#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<opencv2\opencv.hpp>
#define BYTE_MASK 0x0000ff
#define HALF_BYTE_MASK 0x00000f
#define DELTA_STEP 16
#define CHANNELS 3  // B G R
using namespace std;
using namespace cv;
void read_int_from_binary(fstream &, int &);
void create_table(int[][DELTA_STEP], int[], int[]);
bool file_exist(string);
void set_mat_pixel(Mat, fstream &, int[][DELTA_STEP]);
void read_min_max(fstream &, int[], int[], int[]);
bool check_header(fstream &);
int main() {
	string imgname;

	cout << "請輸入欲解壓縮檔的名稱(包含附檔名)\n:";
	cin >> imgname;
	if (!file_exist(imgname)) {
		system("pause");
		return 0;
	}
	//imgname = "binary_file.b";
	fstream file;
	file.open(imgname, ios::in | ios::binary);
	//char buffer[1];
	if (!check_header(file)) {
		cout << "檔案格式有誤" << endl;
		system("pause");
		return 0;
	}
	int cols, rows, max[CHANNELS], min[CHANNELS], delta[CHANNELS];
	int grade[CHANNELS][DELTA_STEP];
	read_int_from_binary(file, rows);
	read_int_from_binary(file, cols);
	cout << rows << endl;
	cout << cols << endl;
	read_min_max(file, min, max, delta);

	create_table(grade, delta, min);
	Mat mat(rows, cols, CV_8UC3, Scalar(200, 100, 0));
	set_mat_pixel(mat, file, grade);
	file.close();
	imshow("結果", mat);
	waitKey(0);
	//system("pause");
	return 0;
}
bool file_exist(string filename) {
	fstream file;
	file.open(filename, ios::in);
	if (file.is_open()) {
		file.close();
		return true;
	}
	cout << "檔案不存在" << endl;
	return false;
}
void read_int_from_binary(fstream &file, int &integer) {
	char buffer[1];
	int temp = 0;

	file.read(buffer, 1);
	temp = buffer[0];
	integer = (temp << 8);

	file.read(buffer, 1);
	temp = buffer[0];
	integer += temp;

	return;
}
void create_table(int grade[][DELTA_STEP], int delta[], int min[]) {
	for (int i = 0; i < CHANNELS; i++) {
		for (int j = 0; j < DELTA_STEP; j++) {
			grade[i][j] = min[i] + j*delta[i];
			cout << grade[i][j] << " ";
		}
		cout << endl;
	}
	return;
}
void read_min_max(fstream &file, int min[], int max[], int delta[]) {
	char buffer[1];
	for (int i = 0; i < CHANNELS; i++) {
		file.read(buffer, 1);
		min[i] = ((int)buffer[0] & BYTE_MASK);
		cout << min[i] << " ";
		file.read(buffer, 1);
		max[i] = ((int)buffer[0] & BYTE_MASK);
		cout << max[i] << endl;

		delta[i] = (max[i] - min[i]) / DELTA_STEP;
	}
	return;
}
void set_mat_pixel(Mat mat, fstream &file, int grade[][DELTA_STEP]) {
	char buffer[1];
	int last_buffer, i = 0, temp_color;
	for (int rows = 0; rows < mat.rows; rows += 2) {
		for (int cols = 0; cols < mat.cols; cols += 2) {

			for (int channels = 0; channels < CHANNELS; channels++) {
				if (i == 0) {
					file.read(buffer, 1);
					temp_color = ((int)(buffer[0] >> 4) & HALF_BYTE_MASK);
					last_buffer = ((int)(buffer[0]) & HALF_BYTE_MASK);
					i++;
				}
				else if (i == 1) {
					temp_color = last_buffer;
					i = 0;
				}
				temp_color = grade[channels][temp_color];
				mat.at<Vec3b>(rows, cols)[channels] = temp_color;
				mat.at<Vec3b>(rows, cols + 1)[channels] = temp_color;
				mat.at<Vec3b>(rows + 1, cols)[channels] = temp_color;
				mat.at<Vec3b>(rows + 1, cols + 1)[channels] = temp_color;
			}
		}
	}
	return;
}
bool check_header(fstream &file) {
	char buffer[3];
	char header[3];
	header[0] = 'F'; header[1] = 'I'; header[2] = 'N';
	file.read(buffer, 3);
	for (int i = 0; i < 3; i++) {
		if (buffer[i] != header[i]) {
			return false;
		}
	}
	return true;
}
