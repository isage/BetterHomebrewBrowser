#include <kernel.h>
#include <paf.h>

#include "pages/page.h"
#include "utils.h"
#include "common.h"
#include "main.h"

using namespace paf;

generic::Page *generic::Page::currPage = SCE_NULL;
ui::Plane *generic::Page::templateRoot = SCE_NULL;

ui::CornerButton *g_backButton;
ui::CornerButton *g_forwardButton;
ui::BusyIndicator *g_busyIndicator;

generic::Page::BackButtonEventCallback generic::Page::backCallback;
void *generic::Page::backData;

generic::Page::Page(const char *pageName)
{
    this->prev = currPage;
    currPage = this;

    Plugin::TemplateInitParam tInit;
    Resource::Element e;

    e.hash = Utils::GetHashById(pageName);
    
    //SceBool isMainThread = thread::IsMainThread();
    //if(!isMainThread && !thread::s_mainThreadMutex.TryLock())
    //    thread::s_mainThreadMutex.Lock();

    mainPlugin->TemplateOpen(templateRoot, &e, &tInit);
    root = (ui::Plane *)templateRoot->GetChildByNum(templateRoot->childNum - 1);

	if (currPage->prev != NULL)
	{
		currPage->prev->root->PlayAnimation(0, ui::Widget::Animation_3D_SlideToBack1);
		if (prev->root->animationStatus & 0x80)
			prev->root->animationStatus &= ~0x80;

		g_backButton->PlayAnimation(0, ui::Widget::Animation_Reset);
        Page::SetBackButtonEvent(NULL, NULL);

		if (currPage->prev->prev != SCE_NULL)
		{
			currPage->prev->prev->root->PlayAnimationReverse(0, ui::Widget::Animation_Reset);
			if (prev->prev->root->animationStatus & 0x80)
				prev->prev->root->animationStatus &= ~0x80;
		}
	}
	else g_backButton->PlayAnimationReverse(0, ui::Widget::Animation_Reset);
	
    g_forwardButton->PlayAnimationReverse(0, ui::Widget::Animation_Reset);

    root->PlayAnimation(-50000, ui::Widget::Animation_3D_SlideFromFront);
	if (root->animationStatus & 0x80)
		root->animationStatus &= ~0x80;

    //if(!isMainThread)
    //    thread::s_mainThreadMutex.Lock();
}

SceVoid generic::Page::OnRedisplay()
{

}

SceVoid generic::Page::OnDelete()
{

}

generic::Page::~Page()
{
	common::Utils::WidgetStateTransition(-100, this->root, ui::Widget::Animation_3D_SlideFromFront, SCE_TRUE, SCE_TRUE);
	if (prev != SCE_NULL)
	{
		prev->root->PlayAnimationReverse(0.0f, ui::Widget::Animation_3D_SlideToBack1);
		prev->root->PlayAnimation(0.0f, ui::Widget::Animation_Reset);
		if (prev->root->animationStatus & 0x80)
			prev->root->animationStatus &= ~0x80;

		if (prev->prev != SCE_NULL) {
			prev->prev->root->PlayAnimation(0.0f, ui::Widget::Animation_Reset);
			if (prev->prev->root->animationStatus & 0x80)
				prev->prev->root->animationStatus &= ~0x80;
		}
	}
	currPage = this->prev;

    if (currPage != NULL && currPage->prev != SCE_NULL)
        g_backButton->PlayAnimation(0, ui::Widget::Animation_Reset);
    else g_backButton->PlayAnimationReverse(0, ui::Widget::Animation_Reset);

    if(currPage != SCE_NULL) currPage->OnRedisplay(); 
}

void generic::Page::Setup()
{
    if(templateRoot) return; //Already Initialised

    Resource::Element e;
    Plugin::PageInitParam pInit;
    
    e.hash = Utils::GetHashById("page_main");
    ui::Widget *page =  mainPlugin->PageOpen(&e, &pInit);
    
    e.hash = Utils::GetHashById("template_plane");
    templateRoot = (ui::Plane *)page->GetChildByHash(&e, 0);

    currPage = SCE_NULL;

    e.hash = Utils::GetHashById("back_button");
    g_backButton = (ui::CornerButton *)page->GetChildByHash(&e, 0);

    backCallback = NULL;
    backData = NULL;

    g_backButton->PlayAnimationReverse(0, ui::Widget::Animation::Animation_Reset);

    ui::Widget::EventCallback *backButtonEventCallback = new ui::Widget::EventCallback();
    backButtonEventCallback->eventHandler = generic::Page::BackButtonEventHandler;
    g_backButton->RegisterEventCallback(ui::Widget::EventMain_Decide, backButtonEventCallback, 0);

    e.hash = Utils::GetHashById("main_busy");
    g_busyIndicator = (ui::BusyIndicator *)page->GetChildByHash(&e, 0);
    g_busyIndicator->Stop();

    e.hash = Utils::GetHashById("forward_button");
    g_forwardButton = (ui::CornerButton *)page->GetChildByHash(&e, 0);
    g_forwardButton->PlayAnimationReverse(0, ui::Widget::Animation_Reset);
}

void generic::Page::SetBackButtonEvent(BackButtonEventCallback callback, void *data)
{
    backCallback = callback;
    backData = data;
}

void generic::Page::BackButtonEventHandler(SceInt32 eventID, ui::Widget *self, SceInt32 unk, ScePVoid)
{
    if(backCallback)
        backCallback(eventID, self, unk, backData);
    else    
        generic::Page::DeleteCurrentPage();
}

void generic::Page::DeleteCurrentPage()
{
	delete currPage;
}