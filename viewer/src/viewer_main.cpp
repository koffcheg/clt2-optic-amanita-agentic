//
// Created by user on 01.09.24.
//
#define CVUI_IMPLEMENTATION
#include "viewer_function.h"
#include <iostream>
#include <filesystem>
#define WINDOW_NAME "Data DP1&DP2"
namespace fs = std::filesystem;

int main(int argc, const char *argv[])
{
    cv::CommandLineParser parser(argc, argv,
								 "{help h||}"
								 "{@dp1meas||path to dp1 measures}"
								 "{@dp2meas||path to dp2 measures}"
								 "{@images||path to images}"
                                 "{@binocular|0|check binocular}"
                                 "{@fps|10|fps for output video}"
                                 );
	parser.about("This program demonstrates the results of processing dp1 and dp2");
	if (parser.has("help") || argc != 6 )
    {
		parser.printMessage();
        return 1;
    }

    std::string path_bin_dp1, path_bin_dp2, path_img, model_data, tmp_str, path_viewer;
    int num = 3, check_binocular, fps;

	if (!(get_arg(parser, 0, path_bin_dp1, "dp1 measures") &&
		  get_arg(parser, 1, path_bin_dp2, "dp2 measures") &&
		  get_arg(parser, 2, path_img, "src. images") &&
          get_arg(parser, 3, check_binocular, "check binocular") &&
          get_arg(parser, 4, fps, "fps"))) {
		parser.printMessage();
		return 1;
	}

    std::vector<std::string> name_files_dp1 = creating_list_files_folder(path_bin_dp1, ".blob");
    std::vector<std::string> name_files_dp2 = creating_list_files_folder(path_bin_dp2, ".blob");
    std::vector<std::string> name_files_im = creating_list_files_folder(path_img, ".png");

    int max_num_file = (int)name_files_im.size()-1;
    cv::Mat im, frame, frame_to_save;
    bool dp1 = true, erase = false, all_video = false;

    // Init a OpenCV window and tell cvui to use it.
    cv::namedWindow(WINDOW_NAME,cv::WINDOW_NORMAL);
    cvui::init(WINDOW_NAME);
    std::vector<int> compression_params;
    Trajectory trajectory;
    int indx_file = 0, tmp_indx_file = 0;
    cv::VideoWriter video;
    while (true) {

		im = cv::imread(compose_file_name(path_img, name_files_im[indx_file], ".png"));
        if (erase)
            frame = im.clone();
        else{
            if (dp1) {
                calc_frame_dp1(im, compose_file_name(path_bin_dp1, name_files_dp1[indx_file], ".blob"),  frame, check_binocular);
                tmp_str = "dp1";
             }
            else {
//                calc_frame_dp2(im, compose_file_name(path_bin_dp2, name_files_dp2[indx_file], ".blob"),  frame);
                tmp_str = "dp2";
                calc_frame_dp2(im, path_bin_dp2, name_files_dp2,indx_file,frame);
            }
            if (all_video){
                tmp_str = "dp1-2";
            }
        }
        frame_to_save = frame.clone();

        // Render the settings window to house the UI
        cvui::window(frame, frame.cols/2 - 200, frame.rows - 300, 400, 300, "Settings");
        cvui::printf(frame, int(im.cols/2)-150, im.rows - 50, 2, 0xff0000, "Frame: %d", indx_file);
        // Checkbox to enable/disable the view dp1/dp2 data
        cvui::checkbox(frame, frame.cols/2 - 190, frame.rows - 250, "DataPro1", &dp1,
                       0xCECECE,0.7);

        cvui::checkbox(frame, frame.cols/2 + 100, frame.rows - 250, "Erase", &erase,
                       0xCECECE,0.7);

        cvui::checkbox(frame, frame.cols/2 - 20, frame.rows - 200, "All video", &all_video,
                       0xCECECE,0.7);

        if (cvui::button(frame, frame.cols/2 - 20, frame.rows - 250, "Video",0.5)) {
            tmp_indx_file = indx_file;
            path_viewer = path_bin_dp1 + "../../viewer/";// + tmp_str + ".mp4";
            if (!std::filesystem::exists(path_viewer)) {
                std::filesystem::create_directories(path_viewer);
            }
//            video.open(path_viewer + tmp_str + ".mp4", cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, cv::Size(frame.cols, frame.rows));
            cv::Mat im2, im_dp1, im_dp2, matRoi, matDst;
            if (all_video) {
                if (im.cols < im.rows) {
                    matDst.create(im.rows, im.cols * 2, im.type());
                    video.open(path_viewer + tmp_str + ".mp4", cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, cv::Size(frame_to_save.cols * 2, frame_to_save.rows));
                } else {
                    matDst.create(im.rows * 2, im.cols , im.type());
                    video.open(path_viewer + tmp_str + ".mp4", cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, cv::Size(frame_to_save.cols, frame_to_save.rows * 2));
                }
            } else
                video.open(path_viewer + tmp_str + ".mp4", cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, cv::Size(frame.cols, frame.rows));

            for (size_t j=0; j<name_files_im.size(); j++){
                std::cout << "Save frame " << j << " to video from " << name_files_im.size() << " frames." << std::endl;
                im = cv::imread(compose_file_name(path_img, name_files_im[j], ".png"));
                if (!all_video){
                    if (dp1) {
                        calc_frame_dp1(im, compose_file_name(path_bin_dp1, name_files_dp1[j], ".blob"), frame,
                                       check_binocular);
                    } else {
                        calc_frame_dp2(im, path_bin_dp2, name_files_dp2, j, frame);
                    }
                    video.write(frame);
                } else {
                    im2 = im.clone();
                    calc_frame_dp1(im, compose_file_name(path_bin_dp1, name_files_dp1[j], ".blob"),
                                   im_dp1,
                                   check_binocular);
                    calc_frame_dp2(im2, path_bin_dp2, name_files_dp2, j, im_dp2);
                    if (im.cols < im.rows) {
                        matRoi = matDst(cv::Rect(0,0,im.cols,im.rows));
                        im_dp1.copyTo(matRoi);
                        matRoi = matDst(cv::Rect(im.cols,0,im.cols,im.rows));
                        im_dp2.copyTo(matRoi);
                    }
                    else {
                        matRoi = matDst(cv::Rect(0,0,im.cols,im.rows));
                        im_dp1.copyTo(matRoi);
                        matRoi = matDst(cv::Rect(0,im.rows,im.cols,im.rows));
                        im_dp2.copyTo(matRoi);
                    }
                    video.write(matDst);
                }
            }
            video.release();
            indx_file = tmp_indx_file;
        }

        // Two trackbars to control the number frames
        cvui::trackbar(frame, frame.cols/2 - 190, frame.rows - 200, 380,
                       &indx_file, 0, max_num_file,
                       1,"%.1Lf",0,1,0.7);

        // Show a button << (110, 80)
        if (cvui::button(frame, frame.cols/2 - 190, frame.rows - 110, "<<",0.7)) {
            // The button was clicked, so let's increment our counter.
            indx_file--;
            if (indx_file<0) indx_file = 0;
        }

        if (cvui::button(frame, frame.cols/2 + 100, frame.rows - 110, ">>",0.7)) {
            // The button was clicked, so let's increment our counter.
            indx_file++;
            if (indx_file>=max_num_file) indx_file = max_num_file;
        }

        // Show a button Screenshot (110, 80)
        if (cvui::button(frame, frame.cols/2 - 60, frame.rows - 110, "Screenshot",0.5)) {
            if (!erase){
                path_viewer = path_bin_dp1 + "../../viewer/" + tmp_str + "/";
                if (!std::filesystem::exists(path_viewer)) {
                    std::filesystem::create_directories(path_viewer);
                }
                compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
                compression_params.push_back(9);
                cv::imwrite(path_viewer + name_files_dp1[indx_file] + ".png", frame_to_save, compression_params);
            }
        }

        // Update cvui internal stuff
        cvui::update();

        // Show everything on the screen
        cv::imshow(WINDOW_NAME, frame);

        // Check if ESC was pressed
        if (cv::waitKey(30) == 27) {
            break;
        }
    }
    return 0;
}
