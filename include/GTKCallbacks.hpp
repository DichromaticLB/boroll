#ifndef _GTKALLBAKS_HPP
#define _GTKALLBAKS_HPP
#ifdef _GTKMAINBOROLL
#include<gtkmm/application.h>
#include<gtkmm/builder.h>
#include<gtkmm/window.h>
#include<gtkmm/frame.h>
#include<gtkmm/menu.h>
#include<gtkmm/menuitem.h>
#include<gtkmm/entry.h>
#include<gtkmm/liststore.h>
#include<gtkmm/treeview.h>
#include<gtkmm/treeviewcolumn.h>
#include<gdkmm/monitor.h>
#include<gtk-3.0/gdk/gdkevents.h>
#include<gtk-3.0/gdk/gdktypes.h>
#include<gtk-3.0/gdk/gdkmonitor.h>
#include<gtk-3.0/gdk/gdkkeysyms.h>
#include<mutex>
#include "ShiftingFrame.hpp"
#include"XMLPostFetch.hpp"

namespace GTKCallbacks
{
	void _showTags(int x,int y,bool getFocus);

	namespace items
	{
		class GtkMenuItemForwardLabel;
		Glib::RefPtr<Gdk::Cursor> defaultCursor;
		Glib::RefPtr<Gdk::Cursor> dragCursor;
		Glib::RefPtr<Gtk::Application> app;
		Glib::RefPtr<Gtk::Builder> builder;

		Gtk::TreeModel::ColumnRecord model;
		Glib::RefPtr<Gtk::ListStore> refTreeModel;
		Gtk::TreeModelColumn<std::string> compRep;
		Gtk::TreeModelColumn<std::string> compValue;

		std::vector<Gtk::MenuItem> filters;
		std::vector<Gtk::Menu> 	   tagGroup;
		std::vector<GtkMenuItemForwardLabel> tags;
		std::vector<std::function<void()>> menuBuildCallbacks;
		std::mutex pauseAccess;
		unsigned char pause;

		Bofiguration::TokenAutocomplete lastAutocomplete;

		ShiftingFrame& dynamicDisplay()
		{
			static ShiftingFrame display;

			return display;
		}

		Gtk::Window* window()
		{
			Gtk::Window* w;
			items::builder->get_widget("topwin",w);
			return w;
		}

		Gtk::Window*  inputPopup()
		{
			Gtk::Window*e;
			 items::builder->get_widget("entryPop",e);
			 return e;
		}

		Gtk::Entry*  input()
		{
			Gtk::Entry*e;
			 items::builder->get_widget("entry",e);
			 return e;
		}

		Gtk::MenuItem *menuExit()
		{
			Gtk::MenuItem * mi;
			items::builder->get_widget("exit",mi);
			return mi;
		}

		Gtk::MenuItem *menuMinimize()
		{
			Gtk::MenuItem *me;
			items::builder->get_widget("minimize",me);
			return me;
		}


		Gtk::MenuItem *menuTags()
		{
			Gtk::MenuItem *me;
			items::builder->get_widget("tags",me);
			return me;
		}


		Gtk::Menu *rMenu()
		{
			Gtk::Menu *m;
			items::builder->get_widget("rightClickMenu", m);
			return m;
		}

		Gtk::Menu *rMenuFilter()
		{
			Gtk::Menu *me;
			items::builder->get_widget("filterMenu",me);
			return me;
		}

		Gtk::Menu *rMenuTags()
		{
			Gtk::Menu *me;
			items::builder->get_widget("tagMenu",me);
			return me;
		}

		Gtk::TreeView* treeCompletion()
		{
			Gtk::TreeView *me;
			items::builder->get_widget("completions",me);
			return me;
		}

		bool moving=false,maxi=false;
		int lastX,lastY;

		class GtkMenuItemForwardLabel: public Gtk::MenuItem{
		public:

			bool clicked(GdkEventButton* e)
			{
				auto i=GTKCallbacks::items::input();
				if(e->button==1)
					i->set_text(i->get_text()+" "+get_label());
				else
					i->set_text(get_label());

				int x,y;
				items::window()->get_position(x,y);
				GTKCallbacks::_showTags(x,y,false);
				return true;
			};

			void setForward()
			{
				signal_button_release_event().connect(sigc::mem_fun(*this,&GtkMenuItemForwardLabel::clicked));
			}
		};
	}

	int run()
	{
		return items::app->run(*items::window());
	}

	 void pause()
	 {
		std::lock_guard<std::mutex> m( items::pauseAccess);
	 	items::pause++;
	 }

	 void unpause()
	 {
		std::lock_guard<std::mutex> m( items::pauseAccess);
	 	if(items::pause)
	 		items::pause--;
	 }

	 bool isPaused()
	 {
		 std::lock_guard<std::mutex> m( items::pauseAccess);
		 return items::pause>0;
	 }

	void fullscreenSwitch()
	{
		if(!items::maxi)
		{
			Gdk::Rectangle r;
			int x,y;
			items::window()->get_position(x,y);
			items::window()->get_window()->get_display()->get_monitor_at_point(x,y)->get_geometry(r);
			items::dynamicDisplay().fullscreen(r.get_width(),r.get_height());
			unpause();
			items::dynamicDisplay().update();
			items::window()->fullscreen();
			items::maxi=true;
			items::dynamicDisplay().queue_draw();
		}
		else
		{
			unpause();
			items::dynamicDisplay().update();
			items::dynamicDisplay().unfullscreen();
			items::window()->unfullscreen();
			items::maxi=false;
		}
	}

	bool clickPressWindowCB(GdkEventButton* e) {
		if (e->type == GDK_BUTTON_PRESS && e->button == 1)
		{
				items::lastX = e->x+items::dynamicDisplay().get_margin_left();
				items::lastY = e->y+items::dynamicDisplay().get_margin_top();
				items::moving = true;
				items::window()->get_window()->set_cursor(items::dragCursor);
				pause();
		}
		else if (e->type == GDK_BUTTON_PRESS && e->button == 3)
		{
			for(auto&cb:items::menuBuildCallbacks)
				cb();

			if (!items::rMenu()->get_attach_widget())
				items::rMenu()->attach_to_widget(*items::window());

			items::rMenu()->popup(e->button, e->time);
		}
		else if (e->type == GDK_2BUTTON_PRESS && e->button == 1)
		{
			fullscreenSwitch();
		}

		return true;
	}

	bool clickReleaseWindowCB(GdkEventButton* e)
	{
		if (e->type == GDK_BUTTON_RELEASE && e->button == 1) {
			items::moving = false;
			unpause();
			items::window()->get_window()->set_cursor(items::defaultCursor);

		}
		return true;
	}


	 bool clickMoveCB(GdkEventMotion*c)
	 {
		  if(items::moving)
			  items::window()->move(c->x_root-items::lastX,c->y_root-items::lastY);

		  return true;
	  };

	 bool minimize(GdkEventButton* e)
	 {
		 if (e->type == GDK_BUTTON_PRESS && e->button == 1)
			 items::window()->iconify();
		 return true;
	 }



	 void _showTags(int x,int y,bool getFocus)
	 {
		 items::inputPopup()->move(x,y);
		 items::inputPopup()->set_keep_above(true);
		 items::inputPopup()->show();
		 if(getFocus)
			 items::input()->grab_focus();
	 }

	bool showTags(GdkEventButton* e)
	{
		 if (e->type == GDK_BUTTON_PRESS&& e->button == 1)
			 _showTags(e->x_root+20,e->y_root,true);

		 return true;
	}

	 bool showEntry(GdkEventFocus* event)
	 {
		 pause();
		 return true;
	 }

	 bool hideEntry(GdkEventFocus* event)
	 {
		 unpause();
		 items::inputPopup()->hide();
	 	 return true;
	 }

	 void _closeGTK()
	 {
		 for(auto& t:items::tags)
		 	t.unset_submenu();

		 items::rMenu()->hide();
		 items::app->remove_window(* items::window());
	 }

	bool closeGTK(GdkEventButton* b)
	{

		if(b->button==1)
			_closeGTK();

		return true;
	}

	unsigned getSelectedCompletion()
	{
		int res=0;
		auto r=items::treeCompletion()->get_selection()->get_selected_rows();
		if(!r.empty())
			res=r.front().front();
		return res;
	}

	void doCompletion()
	{
		auto rows=items::refTreeModel->children();
		auto iter=rows.begin();
		unsigned pos=getSelectedCompletion();
		if(pos<rows.size())
		{
			std::advance(iter,pos);
			std::string completion=(*iter).get_value(items::compValue),subs;
			auto tok=items::lastAutocomplete;
			if(tok.autocIndex!=-1)
				for(unsigned i=0;i<tok.tokens.size();i++)
				{
					subs+=i==((unsigned)tok.autocIndex)?completion:tok.tokens[i];
					subs+=" ";
				}
			items::input()->set_text(subs);
		}
	}

	 void setup(int argc, char *argv[])
	 {
		 for(auto& accepted:Gdk::Pixbuf::get_formats())
		  		Bofiguration::putvec("ValidFormatsVariable",accepted.get_name());

		 items::pause=0;
		 items::app = Gtk::Application::create(argc, argv, "Borroll.instance.n"+Bofiguration::fetch<std::string>("id"));
	 	 items::builder = Gtk::Builder::create_from_file("res/gtklayout.glade");
	 	 items::defaultCursor=Gdk::Cursor::create(items::window()->get_window()->get_display(),Gdk::CursorType::ARROW);
	 	 items::dragCursor=Gdk::Cursor::create(items::window()->get_window()->get_display(),Gdk::CursorType::FLEUR);
	 	 items::window()->set_keep_above(Bofiguration::fetch<bool>("alwaysOnTop"));
	 	 items::window()->set_skip_taskbar_hint(Bofiguration::fetch<bool>("hideInTaskBar"));
	 	 items::window()->set_skip_pager_hint(Bofiguration::fetch<bool>("skipPager"));
	 	 items::window()->set_decorated(Bofiguration::fetch<bool>("decorated"));
		 items::rMenu()->signal_show().connect(&pause);
		 items::rMenu()->signal_hide().connect(&unpause);

     	 items::model.add(items::compRep);
 	 	 items::model.add(items::compValue);
	 	 items::refTreeModel=Gtk::ListStore::create(items::model);
	 	 items::treeCompletion()->set_model(items::refTreeModel);
	 	 items::treeCompletion()->append_column("",items::compRep);

	 	 items::treeCompletion()->signal_row_activated().connect([](const Gtk::TreeModel::Path&p,Gtk::TreeViewColumn* v){
	 		 doCompletion();
	 	 });

	 	 items::treeCompletion()->signal_key_release_event().connect([](GdkEventKey*k)
	 	 {
	 		 if(k->type!=GDK_KEY_RELEASE){}
	 		 else if(k->keyval==GDK_KEY_Escape||((k->keyval&0xDF)==GDK_KEY_C&&(k->state&GDK_CONTROL_MASK))||(k->keyval==GDK_KEY_Up&&getSelectedCompletion()==0))
	 		 {
	 			 items::input()->grab_focus();
	 		 }
	 		 return true;
	 	 });

	 	items::treeCompletion()->signal_key_press_event().connect([](GdkEventKey*k)
	 	{
			 if(k->type!=GDK_KEY_PRESS){}
			 else if(k->keyval==GDK_KEY_Tab)
			 {
				 items::input()->grab_focus();
			 }
			 return true;
	 	});

	 	 std::string tags;
	 	 for(auto tag:Bofiguration::fetchVec("paramGETTagsDefault"))
	 		 tags+=" "+tag;

	 	 items::input()->set_text(tags);

	 	 GTKCallbacks::items::inputPopup()->signal_focus_in_event().connect(&GTKCallbacks::showEntry);
	 	 GTKCallbacks::items::inputPopup()->signal_focus_out_event().connect(&GTKCallbacks::hideEntry);
	 	 GTKCallbacks::items::menuExit()->signal_button_release_event().connect(&GTKCallbacks::closeGTK);
	 	 GTKCallbacks::items::menuMinimize()->signal_button_press_event().connect(&GTKCallbacks::minimize);
	 	 GTKCallbacks::items::menuTags()->signal_button_press_event().connect(&GTKCallbacks::showTags);

	 	 GTKCallbacks::items::input()->signal_focus_in_event().connect([](GdkEventFocus* event){
	 		items::treeCompletion()->hide();
	 		items::treeCompletion()->set_can_focus(false);
	 		items::inputPopup()->set_size_request(items::input()->get_width(),items::input()->get_height());
	 		return true;
	 	 });


	 	 items::dynamicDisplay().set_events(Gdk::EventMask::BUTTON_PRESS_MASK|Gdk::EventMask::BUTTON_RELEASE_MASK|Gdk::EventMask::BUTTON1_MOTION_MASK);
	 	 items::window()->signal_button_press_event().connect(&clickPressWindowCB);
	 	 items::window()->signal_button_release_event().connect(&clickReleaseWindowCB);
	 	 items::window()->signal_motion_notify_event().connect(&clickMoveCB);

	 	 GTKCallbacks::items::window()->add(items::dynamicDisplay());
	 	 GTKCallbacks::items::window()->override_background_color(Gdk::RGBA(Bofiguration::fetch<std::string>("windowBGRGBA")));
	 	 items::tagGroup.reserve(100);
	 	 items::tags.reserve(500);

	 	 GTKCallbacks::items::window()->show_all();
	 	 items::dynamicDisplay().queue_draw();
	 }

};

#endif
#endif
