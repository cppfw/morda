#include "../../shaders/PosShader.hpp"

#include "../../render/FrameBuffer.hpp"

#include "../../Morda.hpp"

#include "../../util/util.hpp"

#include "Widget.hpp"

#include "container/Container.hpp"



using namespace morda;



Widget::Widget(const stob::Node* chain){
	if(const stob::Node* n = getProperty(chain, "layout")){
		this->layout = n->cloneChain();
	}

	if(const stob::Node* n = getProperty(chain, "x")){
		this->rectangle.p.x = morda::dimValueFromSTOB(*n);
	}else{
		this->rectangle.p.x = 0;
	}
	
	if(const stob::Node* n = getProperty(chain, "y")){
		this->rectangle.p.y = morda::dimValueFromSTOB(*n);
	}else{
		this->rectangle.p.y = 0;
	}

	if(const stob::Node* n = getProperty(chain, "dx")){
		this->rectangle.d.x = morda::dimValueFromSTOB(*n);
	}else{
		this->rectangle.d.x = 0;
	}
	
	if(const stob::Node* n = getProperty(chain, "dy")){
		this->rectangle.d.y = morda::dimValueFromSTOB(*n);
	}else{
		this->rectangle.d.y = 0;
	}

	if(const stob::Node* p = getProperty(chain, "name")){
		this->nameOfWidget = p->value();
	}

	if(const stob::Node* p = getProperty(chain, "clip")){
		this->clip_v = p->asBool();
	}else{
		this->clip_v = false;
	}
	
	if(const stob::Node* p = getProperty(chain, "cache")){
		this->cache = p->asBool();
	}else{
		this->cache = false;
	}
	
	if(const stob::Node* p = getProperty(chain, "visible")){
		this->isVisible_v = p->asBool();
	}else{
		this->isVisible_v = true;
	}
	
	if(const stob::Node* p = getProperty(chain, "enabled")){
		this->isEnabled_v = p->asBool();
	}else{
		this->isEnabled_v = true;
	}
}



Widget::LayoutParams::LayoutParams(const stob::Node* chain){
	if(auto n = getProperty(chain, "dx")){
		this->dim.x = real(dimValueFromLayoutStob(*n));
	}else{
		this->dim.x = LayoutParams::min_c;
	}
	
	if(auto n = getProperty(chain, "dy")){
		this->dim.y = real(dimValueFromLayoutStob(*n));
	}else{
		this->dim.y = LayoutParams::min_c;
	}
}



std::shared_ptr<Widget> Widget::findChildByName(const std::string& name)noexcept{
	if(this->name() == name){
		return this->sharedFromThis(this);
	}
	return nullptr;
}



void Widget::resize(const morda::Vec2r& newDims){
	if(this->rectangle.d == newDims){
		if(this->relayoutNeeded){
			this->clearCache();
			this->relayoutNeeded = false;
			this->layOut();
		}
		return;
	}

	this->clearCache();
	this->rectangle.d = newDims;
	utki::clampBottom(this->rectangle.d.x, real(0.0f));
	utki::clampBottom(this->rectangle.d.y, real(0.0f));
	this->relayoutNeeded = false;
	this->onResize();//call virtual method
}



std::shared_ptr<Widget> Widget::removeFromParent(){
	if(!this->parentContainer){
		throw morda::Exc("Widget::RemoveFromParent(): widget is not added to the parent");
	}
	return this->parentContainer->remove(*this);
}



void Widget::setRelayoutNeeded()noexcept{
	if(this->relayoutNeeded){
		return;
	}
	this->relayoutNeeded = true;
	if(this->parentContainer){
		this->parentContainer->setRelayoutNeeded();
	}
	this->cacheTex = Texture2D();
}



void Widget::renderInternal(const morda::Matr4r& matrix)const{
	if(!this->rect().d.isPositive()){
		return;
	}
	
	if(this->cache){
		if(this->cacheDirty){
			bool scissorTestWasEnabled = Render::isScissorEnabled();
			Render::setScissorEnabled(false);

			//check if can re-use old texture
			if(!this->cacheTex || this->cacheTex.dim() != this->rect().d){
				this->cacheTex = this->renderToTexture();
			}else{
				ASSERT(this->cacheTex.dim() == this->rect().d)
				this->cacheTex = this->renderToTexture(std::move(this->cacheTex));
			}
			
			Render::setScissorEnabled(scissorTestWasEnabled);
			this->cacheDirty = false;
		}
		
		//After rendering to texture it is most likely there will be transparent areas, so enable simple blending
		applySimpleAlphaBlending();
		
		this->renderFromCache(matrix);
	}else{
		if(this->clip_v){
	//		TRACE(<< "Widget::RenderInternal(): oldScissorBox = " << Rect2i(oldcissorBox[0], oldcissorBox[1], oldcissorBox[2], oldcissorBox[3]) << std::endl)

			//set scissor test
			kolme::Recti scissor = this->computeViewportRect(matrix);

			kolme::Recti oldScissor;
			bool scissorTestWasEnabled = Render::isScissorEnabled();
			if(scissorTestWasEnabled){
				oldScissor = Render::getScissorRect();
				scissor.intersect(oldScissor);
			}else{
				Render::setScissorEnabled(true);
			}

			Render::setScissorRect(scissor);

			this->render(matrix);

			if(scissorTestWasEnabled){
				Render::setScissorRect(oldScissor);
			}else{
				Render::setScissorEnabled(false);
			}
		}else{
			this->render(matrix);
		}
	}

	//render border
#ifdef M_MORDA_RENDER_WIDGET_BORDERS
	morda::ColorPosShader& s = App::inst().shaders.colorPosShader;
	s.Bind();
	morda::Matr4r matr(matrix);
	matr.scale(this->rect().d);
	s.SetMatrix(matr);

	if(this->isHovered()){
		s.SetColor(kolme::Vec3f(0, 1, 0));
	}else{
		s.SetColor(kolme::Vec3f(1, 0, 1));
	}
	s.render(s.quad01Fan, Shader::EMode::LINE_LOOP);
#endif
}

Texture2D Widget::renderToTexture(Texture2D&& reuse) const {
	Texture2D tex;
	
	if(reuse && reuse.dim() == this->rect().d){
		tex = std::move(reuse);
	}else{
		tex = Texture2D(
				this->rect().d.to<unsigned>(),
				4,
				Render::TexFilter_e::NEAREST,
				Render::TexFilter_e::NEAREST
			);
	}
	
	Render::unbindTexture(0);
	
	FrameBuffer fb;
	
	fb.bind();
	
	fb.attachColor(std::move(tex));
	
	ASSERT(fb.isComplete())
	
	ASSERT_INFO(Render::isBoundFrameBufferComplete(), "tex.dim() = " << tex.dim())
	
	auto oldViewport = Render::getViewport();
	utki::ScopeExit scopeExit([&oldViewport](){
		Render::setViewport(oldViewport);
	});
	
	Render::setViewport(kolme::Recti(kolme::Vec2i(0), this->rect().d.to<int>()));
	
	Render::clearColor(kolme::Vec4f(0.0f));
	
	Matr4r matrix;
	matrix.identity();
	matrix.translate(-1, -1);
	matrix.scale(Vec2r(2.0f).compDivBy(this->rect().d));
	
	this->render(matrix);
	
	tex = fb.detachColor();
	
	fb.unbind();
	
	return tex;
}

void Widget::renderFromCache(const kolme::Matr4f& matrix) const {
	morda::Matr4r matr(matrix);
	matr.scale(this->rect().d);
	
	morda::PosTexShader &s = Morda::inst().shaders.posTexShader;

	ASSERT(this->cacheTex)
	this->cacheTex.bind();
	
	s.setMatrix(matr);
	
	s.render(utki::wrapBuf(morda::PosShader::quad01Fan), utki::wrapBuf(s.quadFanTexCoords));
}

void Widget::clearCache(){
	this->cacheDirty = true;
	if(this->parentContainer){
		this->parentContainer->clearCache();
	}
}


void Widget::onKeyInternal(bool isDown, Key_e keyCode){
	if(this->isInteractive()){
		if(this->onKey(isDown, keyCode)){
			return;
		}
	}

	if(this->parent()){
		this->parent()->onKeyInternal(isDown, keyCode);
	}
}



void Widget::focus()noexcept{
//	ASSERT(App::inst().thisIsUIThread())

	if(this->isFocused()){
		return;
	}

	Morda::inst().setFocusedWidget(this->sharedFromThis(this));
}



void Widget::unfocus()noexcept{
//	ASSERT(App::inst().thisIsUIThread())

	if(!this->isFocused()){
		return;
	}

	ASSERT(Morda::inst().focusedWidget.lock() && Morda::inst().focusedWidget.lock().operator->() == this)

	Morda::inst().setFocusedWidget(nullptr);
}



bool Widget::isTopmost()const noexcept{
	if(!this->parent()){
		return false;
	}
	
	ASSERT(this->parent()->children().size() != 0)
	
	return this->parent()->children().back().get() == this;
}



void Widget::makeTopmost(){
	if(!this->parent()){
		return;
	}
	
	ASSERT(this->parent()->children().size() != 0)
	
	if(this->parent()->children().size() == 1){
		return;
	}
	
	if(this->parent()->children().rbegin()->get() == this){
		return;//already topmost
	}
	
	Container* p = this->parent();
	
	auto w = this->removeFromParent();
	
	p->add(w);
}



kolme::Recti Widget::computeViewportRect(const Matr4r& matrix) const noexcept{
	return kolme::Recti(
			((matrix * Vec2r(0, 0) + Vec2r(1, 1)) / 2).compMulBy(Render::getViewport().d.to<real>()).rounded().to<int>(),
			this->rect().d.to<int>()
		);
}


Vec2r Widget::measure(const morda::Vec2r& quotum) const{
	Vec2r ret(quotum);
	for(unsigned i = 0; i != ret.size(); ++i){
		if(ret[i] < 0){
			ret[i] = 0;
		}
	}
	return ret;
}


Vec2r Widget::calcPosInParent(Vec2r pos, const Widget* parent) {
	if(parent == this || !this->parent()){
		return pos;
	}
	
	ASSERT(this->parent())
	
	return this->parent()->calcPosInParent(this->rect().p + pos, parent);
}


Widget::LayoutParams& Widget::getLayoutParams() {
	if(!this->parent()){
		throw morda::Exc("Widget::getLayoutParams(): widget is not added to any container, cannot get layout params. In order to get layout params the widget should be added to some container.");
	}
	
	return this->parent()->getLayoutParams(*this);
}


const Widget::LayoutParams& Widget::getLayoutParams()const {
	if(!this->parent()){
		throw morda::Exc("Widget::getLayoutParams(): widget is not added to any container, cannot get layout params. In order to get layout params the widget should be added to some container.");
	}
	
	return this->parent()->getLayoutParams(*this);
}
