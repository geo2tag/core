package ru.spb.osll.web.client.ui.core;

import java.util.List;

import ru.spb.osll.web.client.ui.core.SimpleMenuTree.GroupItem;
import ru.spb.osll.web.client.ui.core.SimpleMenuTree.MenuItem;

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.resources.client.ImageResource;
import com.google.gwt.user.client.ui.Anchor;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratedStackPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.user.client.ui.Widget;

abstract public class SimpleMenu extends Composite {

	private SimpleMenuTree m_menuTree = new SimpleMenuTree();
	
	private DecoratedStackPanel m_stackPanel;

	abstract protected void initMenu();

	abstract protected void setContentWidget(SimpleComposite widget);
	
	public SimpleMenu() {
		initMenu();
		initWidget(onInitialize());
	}
	
	public SimpleMenuTree getMenuTree(){
		return m_menuTree;
	}

	public void setSecectedGroup(Widget w){
		int index = m_stackPanel.getSelectedIndex();
		final List<GroupItem> groups = m_menuTree.getGroups();
		for (int i = 0; i < groups.size(); i++){
			for (MenuItem item : groups.get(i).getItems()){
				if (item.getWidget() == w){
					index = i;
				}
			}
		}
		m_stackPanel.showStack(index);
	}

	public String getGroup(Widget w){ 
		final List<GroupItem> groups = m_menuTree.getGroups();
		for (GroupItem group : groups){
			for (MenuItem item : group.getItems()){
				if (item.getWidget() == w){
					return group.getName();
				}
			}
		}
		return null;
	}
	
	public Widget onInitialize() {
		m_stackPanel = new DecoratedStackPanel();
		m_stackPanel.setWidth("224px");
		for (GroupItem group : m_menuTree.getGroups()){
			final boolean containtPicture = group.getImage() != null;
			final String header = containtPicture ? 
					getHeaderString(group.getName(), group.getImage()): group.getName();
			m_stackPanel.add(createItemsWidget(group), header, containtPicture);
		}
		m_stackPanel.ensureDebugId("stackPanel");
		return m_stackPanel;
	}	
		
	private String getHeaderString(String text, ImageResource image) {
		HorizontalPanel hPanel = new HorizontalPanel();
		hPanel.setSpacing(6);
		hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
		if (image != null){
			hPanel.add(new Image(image));
		}
		HTML headerText = new HTML(text);
		hPanel.add(headerText);
		return hPanel.getElement().getString();
	}
	
	private VerticalPanel createItemsWidget(GroupItem group) {
		VerticalPanel panel = new VerticalPanel();
		for (MenuItem item : group.getItems()) {
			final SimpleComposite widget = item.getWidget();
			ClickHandler handler = new ClickHandler() {
				public void onClick(ClickEvent event) {
					setContentWidget(widget);
				}
			};
			
			final HorizontalPanel imlink = new HorizontalPanel();
			imlink.setSpacing(6);

			if (item.getImage() != null){
				final Image im = new Image(item.getImage());
				im.addClickHandler(handler);
				imlink.add(im);
			}
			final Anchor link = new Anchor(item.getName());
			link.addClickHandler(handler);
			imlink.add(link);
			
			panel.add(imlink);
		}
		return panel;
	}
}
