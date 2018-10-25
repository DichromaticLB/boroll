#ifdef _GTKMAINBOROLL
#ifndef INCLUDE_SHIFTINGFRAME_HPP_
#define INCLUDE_SHIFTINGFRAME_HPP_
#include <gtkmm/drawingarea.h>
#include<cairomm-1.0/cairomm/cairomm.h>
#include<string>
#include<functional>
class ShiftingFrame: public  Gtk::DrawingArea
{
public:
	using waitPauses=std::function<bool()>;


	ShiftingFrame();
	bool setImg(const std::string& imgsrc);
	void update();
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
	virtual ~ShiftingFrame()=default;
	void fullscreen  (int w,int h);
	void unfullscreen();
	void setPauseCheck(const waitPauses&w);
	enum
		{
			FIXED,
			EXPAND_FIX,
			EXPAND_EXTEND,
			NO_CHANGE
		};

private:
	waitPauses wait;
	std::string imageSource;
	bool _update,fsmode,keepMargins;
	Glib::RefPtr<Gdk::Pixbuf> imagePixBuf;
	int maxWres,maxHres,fullScreenWres,fullScreenHres;
	int display_mode;
private:
};


#endif /* INCLUDE_SHIFTINGFRAME_HPP_ */
#endif
