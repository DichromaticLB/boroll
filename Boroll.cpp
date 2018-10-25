#ifdef _GTKMAINBOROLL
#include"include/BoAppLoop.hpp"
#include"include/GTKCallbacks.hpp"

int main (int argc, char *argv[])
{

  if(argc>=3&&std::string("--config-file")==argv[1])
  {
		Bofiguration::deff(argv[2]);
		argc-=3;
		argv+=3;
  }

  Bofiguration::setup();
  GTKCallbacks::setup(argc,argv);

  BoAppLoop mainL([](){return GTKCallbacks::items::input()->get_text();},
		   	   	  [](const std::string& src){return GTKCallbacks::items::dynamicDisplay().setImg(src);},
				  &GTKCallbacks::isPaused);

  GTKCallbacks::items::dynamicDisplay().setPauseCheck(&GTKCallbacks::isPaused);
  Glib::ustring previous;
  GTKCallbacks::items::input()->signal_key_release_event().connect(
		  [&](GdkEventKey* k)
		  {
		  	  if(k->keyval==GDK_KEY_Return)
		  	  {
		  		  GTKCallbacks::items::inputPopup()->hide();
		  		  mainL.stop();
  				  mainL.start();

		  	  }
		  	 else if(k->keyval==GDK_KEY_Escape||((k->keyval&0xDF)==GDK_KEY_C&&(k->state&GDK_CONTROL_MASK)))
		  	 {
		  		GTKCallbacks::items::inputPopup()->hide();
		  	  }
		  	  else if(k->keyval==GDK_KEY_Down)
		  	  {

		  		  auto input=GTKCallbacks::items::input();
		  		  auto tok=(GTKCallbacks::items::lastAutocomplete=Bofiguration::tokenize(input->get_text(),input->get_position()));

		  		  if(tok.autocIndex!=-1&&tok.tokens[tok.autocIndex]!=previous)
		  		  {
		  			  previous=tok.tokens[tok.autocIndex];
					  GTKCallbacks::items::refTreeModel->clear();
					  auto completions=XMLAutocomplete::fetch(previous);

					  for(auto&c:completions)
					  {
						  Gtk::TreeModel::Row row = *(GTKCallbacks::items::refTreeModel->append());

						  row[GTKCallbacks::items::compRep] = c;
						  row[GTKCallbacks::items::compValue] = c.val;
					  }
		  		  }

		  		unsigned spawn=GTKCallbacks::items::refTreeModel->children().size();
		  		GTKCallbacks::items::inputPopup()->set_size_request(input->get_width(),spawn*input->get_height());

		  		if(spawn)
		  		{
		  			GTKCallbacks::items::treeCompletion()->set_can_focus(true);
		  			GTKCallbacks::items::treeCompletion()->grab_focus();

		  		}
		  		GTKCallbacks::items::treeCompletion()->show_all();


		  	  }
	       return false;
		  });

  GTKCallbacks::items::window()->signal_key_release_event().connect(
		  [&](GdkEventKey* k){
			if(k->keyval==GDK_KEY_Left)
			{
				mainL.requestBacktrack();
			}
			else if(k->keyval==GDK_KEY_Right)
			{
				mainL.forward();
			}
			else if(((k->keyval&0xDF)==GDK_KEY_S&&(k->state&GDK_CONTROL_MASK)))
			{
				 int x,y;
				 GTKCallbacks::items::window()->get_position(x,y);
				 GTKCallbacks::_showTags(x,y,true);
			}
			else if(((k->keyval&0xDF)==GDK_KEY_W&&(k->state&GDK_CONTROL_MASK)))
			{
				GTKCallbacks::_closeGTK();
			}
			else if(((k->keyval&0xDF)==GDK_KEY_F&&(k->state&GDK_CONTROL_MASK)))
			{
				GTKCallbacks::fullscreenSwitch();
			}

			return true;
		});

  auto defaults=Bofiguration::fetchVec("defaultRules");
  GTKCallbacks::items::filters.reserve(XMLPostFilter::getAllNames().size());

  auto names=XMLPostFilter::getAllNames();
  std::sort(names.begin(),names.end());

  for(auto& filterName:names)
  {
  		GTKCallbacks::items::filters.push_back(Gtk::MenuItem(filterName));
  		Gtk::MenuItem* menuItem=&GTKCallbacks::items::filters.back();
  		GTKCallbacks::items::rMenuFilter()->add(*menuItem);
  		menuItem->set_visible(true);

  		if(std::find(defaults.begin(),defaults.end(),filterName)==defaults.end())
  			menuItem->set_opacity(0.5);
  		else
  			menuItem->set_opacity(1);

  		menuItem->signal_button_release_event().connect([menuItem,&mainL](GdkEventButton* e){
  			 if (e->type == GDK_BUTTON_RELEASE&& e->button == 1)
  			 {
				if(menuItem->get_opacity()<0.75)
				{
					mainL.setFilter(menuItem->get_label());
					menuItem->set_opacity(1);
				}
				else
				{
					mainL.disableFilter(menuItem->get_label());
					menuItem->set_opacity(0.5);
				}
				mainL.filterUpdate();
  			 }
  			return true;
  		});
  }

  std::function<void()> setTags=[&](){

	 	 auto post=mainL.currentPost();
		 char tagSeparator=Bofiguration::_fetch("tagSeparator")[0];
		 auto menu=GTKCallbacks::items::rMenuTags();

		 for(auto c:menu->get_children())
		 {
			 ((Gtk::MenuItem*)c)->unset_submenu();
			 menu->remove(*c);

		 }
		 GTKCallbacks::items::tags.clear();
		 GTKCallbacks::items::tagGroup.clear();

		 for(auto&dtag:Bofiguration::fetchVec("descriptionTags"))
		 {
			 if(GTKCallbacks::items::tags.size()==GTKCallbacks::items::tags.capacity()||
				GTKCallbacks::items::tagGroup.size()==GTKCallbacks::items::tagGroup.capacity()) //Dont support more than 100 categories or 500 tags
				 continue;

			 GTKCallbacks::items::tags.emplace_back();
			 GTKCallbacks::items::tagGroup.emplace_back();
			 GTKCallbacks::items::tags.back().set_label(dtag);
			 GTKCallbacks::items::tags.back().set_submenu(GTKCallbacks::items::tagGroup.back());
			 menu->add(GTKCallbacks::items::tags.back());

			 if(post.existsNoEmpty(dtag))
			 {
				std::stringstream ss(post[dtag]);
				std::string tag;
				while(std::getline(ss,tag,tagSeparator))
				{
					GTKCallbacks::items::tags.emplace_back();
					GTKCallbacks::items::tags.back().set_label(tag);
					GTKCallbacks::items::tagGroup.back().add(GTKCallbacks::items::tags.back());
					GTKCallbacks::items::tags.back().setForward();
				}
			 }
		 }
		 menu->show_all();
	  return true;
  };

  GTKCallbacks::items::menuBuildCallbacks.push_back(setTags);

  GTKCallbacks::items::rMenu()->signal_hide().connect([&](){
	  if(mainL.needUpdate())
	  {
		  mainL.filterUpdateFinish();
		  mainL.stop();
		  mainL.start();
	  }
  });

  mainL.start();

  auto res=GTKCallbacks::run();

  mainL.stop();
  mainL.cleanup();

  return res;
}
#endif
