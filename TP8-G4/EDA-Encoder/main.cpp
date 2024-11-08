#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include "lodepng.h"
#include "imgui.h"
#include "imgui_impl_allegro5.h"
#include "QuadTree.h"

using namespace std;
using namespace lodepng;
using namespace boost::filesystem;

int main(int argc, char* argv[]) {

	// Setup Allegro
	al_init();
	al_install_keyboard();
	al_install_mouse();
	al_init_primitives_addon();
	al_init_image_addon();
	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	ALLEGRO_DISPLAY* display = al_create_display(1000, 600);
	al_set_window_title(display, "EDA Encoder");
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplAllegro5_Init(display);

	bool show_demo_window = true;
	bool show_another_window1 = false;
	bool compress = false;
	bool selectAll = false;
	bool selectNone = false;
	bool finish = false;
	bool windows[1000];
	//const char* labels[1000];
	string labels[1000];
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool checkbox_A_selected = false;
	int tolerancia = 50;

	// Main loop
	static char oldpath[256] = {};
	vector<ALLEGRO_BITMAP*> bitmapVector;
	bool running = true;
	for (int i = 0; i < 1000; i++) {
		windows[i] = false;
	}
	while (running)
	{
		ALLEGRO_EVENT ev;
		while (al_get_next_event(queue, &ev))
		{
			ImGui_ImplAllegro5_ProcessEvent(&ev);
			if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
				running = false;
			if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
			{
				ImGui_ImplAllegro5_InvalidateDeviceObjects();
				al_acknowledge_resize(display);
				ImGui_ImplAllegro5_CreateDeviceObjects();
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplAllegro5_NewFrame();
		ImGui::NewFrame();

		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("EDA Encoder");

		static char newpath[256] = {};
		int i = 0;
		ImGui::InputText("Ingresar Directorio", newpath, sizeof(newpath));

		ImGui::Separator();
		selectAll = ImGui::Button("SelectAll");
		ImGui::SameLine();
		selectNone = ImGui::Button("SelectNone");
		ImGui::Separator();
		ImGui::BeginChild("Imagenes", ImVec2(500, 400));

		if (strcmp(newpath, oldpath) != 0) {
			strcpy_s(oldpath, newpath);
			path p(newpath);
			bitmapVector.clear();
			if (exists(p) && is_directory(p)) {
				for (directory_iterator it{ p }; it != directory_iterator{}; it++) {
					if ((*it).path().extension().string() == ".png") {
						string pathname = (*it).path().string();
						labels[i++] = (*it).path().filename().string();
						bitmapVector.push_back(al_load_bitmap(pathname.c_str()));
					}
				}
			}
		}

		if (selectAll) {
			for (int j = 0; j < bitmapVector.size(); j++)
				windows[j] = true;
		}

		if (selectNone) {
			for (int j = 0; j < bitmapVector.size(); j++)
				windows[j] = false;
		}

		ImGui::Columns(3, "Imagenes", false);
		for (int i = 0; i < bitmapVector.size(); i += 3) {
			ImGui::Image(bitmapVector[i], ImVec2(100, 100));
			ImGui::Checkbox(labels[i].c_str(), &(windows[i]));
		}
		ImGui::NextColumn();
		for (int i = 1; i < bitmapVector.size(); i += 3) {
			ImGui::Image(bitmapVector[i], ImVec2(100, 100));
			ImGui::Checkbox(labels[i].c_str(), &(windows[i]));
		}
		ImGui::NextColumn();
		for (int i = 2; i < bitmapVector.size(); i += 3) {
			ImGui::Image(bitmapVector[i], ImVec2(100, 100));
			ImGui::Checkbox(labels[i].c_str(), &(windows[i]));
		}

		ImGui::Columns();
		ImGui::EndChild();
		ImGui::Separator();
		ImGui::SliderInt("Tolerancia", &tolerancia, 0, 100);
		ImGui::NewLine();
		
		if (compress) {
			compress = false;
			//Hacer la compresion
			for (int i = 0; i < bitmapVector.size(); i++) {
				if (windows[i] == true) {
					vector<unsigned char> imageVector = encodeQuadTree(bitmapVector[i], tolerancia);
					string outputName = labels[i].substr(0, labels[i].length() - 3);
					saveEncodedImage(imageVector, outputName + "eda");
				}
			}
			finish = true;
		}
		
		if (ImGui::Button("Comenzar compresion")) {
			compress = true;
			ImGui::NewLine();
			ImGui::Text("Comprimiendo..");
		}
		if(finish){
			ImGui::NewLine();
			ImGui::Text("Operacion finalizada");
			ImGui::NewLine();
			if (ImGui::Button("Salir"))
				running = false;
		}
		ImGui::End();

		// Rendering
		ImGui::Render();
		al_clear_to_color(al_map_rgba_f(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
		ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
		al_flip_display();
	}

	// Cleanup
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
	al_destroy_event_queue(queue);
	al_destroy_display(display);

	return 0;

}