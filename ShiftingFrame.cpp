#ifdef _GTKMAINBOROLL

#include "include/ShiftingFrame.hpp"

#include"include/Bofiguration.hpp"
#include<gdkmm.h>
#include<gdkmm/pixbuf.h>
#include<gtkmm/container.h>
#include<iostream>
#include<math.h>


ShiftingFrame::ShiftingFrame():imageSource(Bofiguration::fetch< std::string>("welcome")),_update(true){
	maxWres=Bofiguration::fetch< int>("maxw");
	maxHres=Bofiguration::fetch< int>("maxh");
	keepMargins=Bofiguration::fetch< bool>("keepMargins");
	fullScreenWres=0;
	fullScreenHres=0;
	fsmode=false;
	auto dm=Bofiguration::fetch< std::string>("displayMode");

	if(!dm.compare("FIXED"))
		display_mode=FIXED;
	else if(!dm.compare("EXPAND-FIX"))
		display_mode=EXPAND_FIX;
	else if(!dm.compare("EXPAND-EXTEND"))
		display_mode=EXPAND_EXTEND;
	else
		display_mode=NO_CHANGE;
}

static Gdk::InterpType getInterpolationMode() {
	Gdk::InterpType interpType;

	auto mode = Bofiguration::fetch<std::string>("interpolationMode");
	if (mode.compare("NEAREST"))
		interpType = Gdk::InterpType::INTERP_NEAREST;
	else if (mode.compare("TILES"))
		interpType = Gdk::InterpType::INTERP_TILES;
	else if (mode.compare("HYPER"))
		interpType = Gdk::InterpType::INTERP_HYPER;
	else
		interpType = Gdk::InterpType::INTERP_BILINEAR;
	return interpType;
}

static void reescale(Glib::RefPtr<Gdk::Pixbuf> &imagePixBuf,  int maxWres, int maxHres, int mode)
{

	std::double_t woverflow=((double)maxWres)/imagePixBuf->get_width() ;
	std::double_t hoverflow=((double)maxHres)/imagePixBuf->get_height();

	if(mode==ShiftingFrame::EXPAND_EXTEND)
		hoverflow=hoverflow<woverflow?woverflow:hoverflow;
	else if(mode==ShiftingFrame::EXPAND_FIX)
		hoverflow=hoverflow<woverflow?hoverflow:woverflow;

	imagePixBuf=imagePixBuf->scale_simple(
			imagePixBuf->get_width()*((mode==ShiftingFrame::FIXED)?woverflow:hoverflow),
			imagePixBuf->get_height()*hoverflow,
			getInterpolationMode());

}

bool ShiftingFrame::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	try{

		if (_update&&(!wait||!wait()))
		{
			_update=false;
			imagePixBuf=Gdk::Pixbuf::create_from_file(imageSource);
			if(imagePixBuf)
			{
				int _w=fsmode?fullScreenWres:maxWres;
				int _h=fsmode?fullScreenHres:maxHres;

				if(display_mode!=ShiftingFrame::NO_CHANGE)
					reescale(imagePixBuf,_w,_h,display_mode);


				if((fsmode||keepMargins)&&imagePixBuf->get_width()<_w)
				{
					set_margin_left((_w-imagePixBuf->get_width())/2);
					set_margin_right((_w-imagePixBuf->get_width())/2);
				}
				else
				{
					set_margin_left(0);
					set_margin_right(0);
				}

				if((fsmode||keepMargins)&&imagePixBuf->get_height()<_h)
				{
					set_margin_top((_h-imagePixBuf->get_height())/2);
					set_margin_bottom((_h-imagePixBuf->get_height())/2);
				}
				else
				{
					set_margin_top(0);
					set_margin_bottom(0);
				}
			}
		}
		set_size_request(imagePixBuf->get_width(),imagePixBuf->get_height());
		Gdk::Cairo::set_source_pixbuf(cr, imagePixBuf, 0, 0);
		cr->paint();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Failed to open " << imageSource <<" err: "<<e.what()<< std::endl;
	}

	return false;//Resize Parent
}

bool ShiftingFrame::setImg(const std::string& imgsrc)
{
	if(Bofiguration::fileExists(imgsrc))
	{
		_update=true;
		imageSource=imgsrc;
		queue_draw();
		return true;
	}
	return false;
}

void ShiftingFrame::fullscreen (int w,int h)
{

	fsmode=true;
	fullScreenWres=w;
	fullScreenHres=h;
}

void ShiftingFrame::unfullscreen()
{
	fsmode=false;
}

void ShiftingFrame::update()
{
	_update=true;
}

void ShiftingFrame::setPauseCheck(const waitPauses&w)
{
	wait=w;
}
#endif
