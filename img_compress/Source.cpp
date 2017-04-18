#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<opencv2\opencv.hpp>
#define DELTA_STEP 16
#define CHANNELS 3  // B G R
using namespace std;
using namespace cv;
void find_min_max_delta(Mat, int[], int[], int[]);
void create_table(int[][DELTA_STEP], int[], int[]);
void create_file(Mat, int[][DELTA_STEP], int[], int[], string);
int which_step(int, int[][DELTA_STEP], int);
bool file_exist(string);
void output_int_to_binary(fstream &, int);
long long get_file_size(string);
void set_header(fstream &);
int main() {
	char imgname[256];
	cout << "請輸入欲壓縮圖檔名稱(包含附檔名)\n:";
	cin >> imgname;
	if (!file_exist(imgname)) {
		system("pause");
		return 0;
	}
	cv::Mat mat = cv::imread(imgname, 1);
	if (mat.empty())//判斷圖片讀取是否成功
	{
		cout << "圖片存取失敗" << endl;
		system("pause");
		return 0;
	}
	IplImage *img = &IplImage(mat);
	int level = DELTA_STEP, delta[CHANNELS];
	int min[CHANNELS] = { 255.255,255 }, max[CHANNELS] = {}, grade[CHANNELS][DELTA_STEP] = {};

	cout << "cols=" << mat.cols << ",rows=" << mat.rows << endl;
	cout << "m=" << level << endl;
	find_min_max_delta(mat, min, max, delta);
	create_table(grade, delta, min);
	fstream compression;
	compression.open("logo.comtxt", ios::out);
	compression.close();
	compression << mat.rows << endl;
	compression << mat.cols << endl;
	string filename = "result.b";
	create_file(mat, grade, min, max, filename);
	cout << "輸出完成" << endl;
	cout << "原始檔案大小:" << get_file_size(imgname) << " byte" << endl;
	cout << "壓縮後大小:" << get_file_size("result.b") << " byte" << endl;
	cout << "壓縮比:" << get_file_size(imgname) / get_file_size("result.b") << endl;
	cv::imshow("原始", mat);
	waitKey(0);
	//system("pause");
	return 0;
}
void find_min_max_delta(Mat mat, int min[], int max[], int delta[]) {
	int temp;
	IplImage *img = &IplImage(mat);
	for (int rows = 0; rows < mat.rows; rows++) {
		for (int cols = 0; cols < mat.cols; cols++) {
			for (int channels = 0; channels < mat.channels(); channels++) {
				temp = (int)cvGet2D(img, rows, cols).val[channels];
				if (temp < min[channels])min[channels] = temp;
				if (temp > max[channels])max[channels] = temp;
			}
		}
	}
	for (int channels = 0; channels < mat.channels(); channels++) {
		delta[channels] = (max[channels] - min[channels]) / DELTA_STEP;
		cout << min[channels] << " " << max[channels] << " " << delta[channels] << endl;
	}
	return;
}
void create_table(int grade[][DELTA_STEP], int delta[], int min[]) {
	int size = 3;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < DELTA_STEP; j++) {
			grade[i][j] = min[i] + j*delta[i];
			cout << grade[i][j] << " ";
		}
		cout << endl;
	}
	return;
}
void create_file(Mat mat, int grade[][DELTA_STEP], int min[], int max[], string filename) {
	int temp, temp_color;
	char temp_char;
	Mat test(mat.rows, mat.cols, CV_8UC3, Scalar(200, 100, 0));
	IplImage *img = &IplImage(mat);
	fstream file;
	file.open(filename, ios::out | ios::binary);
	set_header(file);

	output_int_to_binary(file, mat.rows);
	output_int_to_binary(file, mat.cols);

	for (int channels = 0; channels < mat.channels(); channels++) {
		temp_char = (char)min[channels];
		file.put(temp_char);
		temp_char = (char)max[channels];
		file.put(temp_char);
	}
	int count = 0, output_data = 0;
	temp_char = temp_char & 0;
	for (int rows = 0; rows < mat.rows; rows += 2) {
		for (int cols = 0; cols < mat.cols; cols += 2) {
			for (int channels = 0; channels < mat.channels(); channels++) {
				temp = (int)cvGet2D(img, rows, cols).val[channels];
				count++;
				count %= 2;
				output_data = which_step(temp, grade, channels);
				if (count == 1) {
					temp_char = ((char)output_data) << 4;
				}
				else {
					temp_char += output_data;
					file.put(temp_char);
					temp_char = 0;
				}

				temp_color = grade[channels][output_data];

				test.at<Vec3b>(rows, cols)[channels] = temp_color;
				test.at<Vec3b>(rows, cols + 1)[channels] = temp_color;
				test.at<Vec3b>(rows + 1, cols)[channels] = temp_color;
				test.at<Vec3b>(rows + 1, cols + 1)[channels] = temp_color;

			}
		}
	}
	file.close();
	imshow("壓縮測試", test);
}
int which_step(int pixel, int grade[][DELTA_STEP], int channel) {
	for (int step = 0; step < DELTA_STEP; step++)
		if (pixel < grade[channel][step])
			return step - 1;
	return DELTA_STEP - 1;
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
void output_int_to_binary(fstream &file, int integer) {
	int temp;
	char input_data;
	temp = integer;
	input_data = (char)(temp >> 8);
	file.put(input_data);
	input_data = (char)temp;
	file.put(input_data);
}
long long get_file_size(string filename) {
	ifstream file(filename, ifstream::in | ifstream::binary);
	// 2. Seek 檔案頭到尾
	file.seekg(0, ios::end);
	// 3. tell 位置 
	long long length = file.tellg();
	// 4. 關檔
	file.close();
	return length;
}
void set_header(fstream &file) {
	char buffer;
	buffer = 'F';
	file.put(buffer);
	buffer = 'I';
	file.put(buffer);
	buffer = 'N';
	file.put(buffer);
	return;
}