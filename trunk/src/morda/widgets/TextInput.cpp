#include "TextInput.hpp"

#include "../App.hpp"

#include <ting/util.hpp>


#if M_OS == M_OS_WINDOWS
#	ifdef DELETE
#		undef DELETE
#	endif
#endif



using namespace morda;



namespace{
std::uint32_t D_CursorBlinkPeriod = 500; //milliseconds

real D_CursorWidth = real(2.0);
}



TextInput::TextInput(const stob::Node* desc) :
		Widget(desc),
		SingleLineTextWidget(desc)
{
	this->SetClip(true);
	
	if(!desc){
		return;
	}
}



void TextInput::Render(const morda::Matr4r& matrix) const{
	{
		morda::Matr4r matr(matrix);
		matr.Translate(this->TextBoundingBox().p.x + this->xOffset, this->Font().BoundingBox().p.y);
		
		PosTexShader& s = [this]() -> PosTexShader&{
			if(this->Color() == 0xffffffff){//if white
				return morda::App::Inst().Shaders().posTexShader;
			}else{
				ColorPosTexShader& s = morda::App::Inst().Shaders().colorPosTexShader;
				s.Bind();
				s.SetColor(this->Color());
				return s;
			}
		}();
		
		ASSERT(this->firstVisibleCharIndex <= this->Text().size())
		this->Font().RenderString(
				s,
				matr,
				ting::Buffer<const std::uint32_t>(
						&*(this->Text().begin() + this->firstVisibleCharIndex),
						this->Text().size() - this->firstVisibleCharIndex
					)
			);
	}
	
	if(this->IsFocused() && this->cursorBlinkVisible){
		morda::Matr4r matr(matrix);
		matr.Translate(this->cursorPos, 0);
		matr.Scale(Vec2r(D_CursorWidth, this->Font().Size()));

		ColorPosShader& s = App::Inst().Shaders().colorPosShader;
		s.Bind();
		s.SetColor(0xffffffff);

		s.SetMatrix(matr);
		s.Render(s.quad01Fan, Shader::EMode::TRIANGLE_FAN);
	}
}

bool TextInput::OnMouseButton(bool isDown, const morda::Vec2r& pos, EMouseButton button, unsigned pointerId){
	if(!isDown){
		return false;
	}
	
	if(button != EMouseButton::LEFT){
		return false;
	}
	
	this->SetCursor(pos.x);
	
	return true;
}

Vec2r TextInput::ComputeMinDim()const NOEXCEPT{
	return Vec2r(this->SingleLineTextWidget::ComputeMinDim().x + D_CursorWidth, this->Font().Size());
}

void TextInput::SetCursorIndex(size_t index){
	this->cursorIndex = index;
	
	ting::util::ClampTop(this->cursorIndex, this->Text().size());
	
	ting::util::ScopeExit scopeExit([this](){
		if(!this->IsFocused()){
			this->Focus();
		}
		this->StartCursorBlinking();
	});
	
	if(this->cursorIndex <= this->firstVisibleCharIndex){
		this->firstVisibleCharIndex = this->cursorIndex;
		this->xOffset = 0;
		this->cursorPos = 0;
		return;
	}
	
	ASSERT(this->firstVisibleCharIndex <= this->Text().size())
	ASSERT(this->cursorIndex > this->firstVisibleCharIndex)
	this->cursorPos = this->Font().StringAdvance(ting::Buffer<const std::uint32_t>(
			&*(this->Text().begin() + this->firstVisibleCharIndex),
			this->cursorIndex - this->firstVisibleCharIndex
		)) + this->xOffset;
	
	ASSERT(this->cursorPos >= 0)
	
	if(this->cursorPos > this->Rect().d.x - D_CursorWidth){
		this->cursorPos = this->Rect().d.x - D_CursorWidth;
		
		this->xOffset = this->cursorPos;//start from rightmost cursor position
		this->firstVisibleCharIndex = this->cursorIndex;
		
		//calculate advance backwards
		for(auto i = this->Text().rbegin() + (this->Text().size() - this->cursorIndex);
				this->xOffset > 0;
				++i
			)
		{
			ASSERT(i != this->Text().rend())
			this->xOffset -= this->Font().CharAdvance(*i);
			ASSERT(this->firstVisibleCharIndex > 0)
			--this->firstVisibleCharIndex;
		}
	}
}



void TextInput::SetCursor(real toPos){
	this->cursorPos = this->xOffset;
	this->cursorIndex = this->firstVisibleCharIndex;
	
	for(auto i = this->Text().begin() + this->firstVisibleCharIndex; i != this->Text().end(); ++i){
		real w = this->Font().CharAdvance(*i);
		
		if(toPos < this->cursorPos + w){
			if(toPos < this->cursorPos + w / 2){
				break;
			}
			this->cursorPos += w;
			++this->cursorIndex;
			break;
		}
		
		this->cursorPos += w;
		++this->cursorIndex;
		
	}
	
	if(this->cursorPos < 0){
		this->cursorPos = 0;
		this->xOffset = 0;
	}
	
//	TRACE(<< "this->cursorPos = " << this->cursorPos << std::endl)
	
	this->cursorBlinkVisible = true;
	
	if(!this->IsFocused()){
		this->Focus();
	}
	this->StartCursorBlinking();
}



void TextInput::Update(std::uint32_t dt){
	this->cursorBlinkVisible = !this->cursorBlinkVisible;
}

void TextInput::OnFocusedChanged(){
	if(this->IsFocused()){
		this->ctrlPressed = false;
		this->StartCursorBlinking();
	}else{
		this->StopUpdating();
	}
}

void TextInput::StartCursorBlinking(){
	this->StopUpdating();
	this->cursorBlinkVisible = true;
	this->StartUpdating(D_CursorBlinkPeriod);
}

bool TextInput::OnKey(bool isDown, EKey keyCode){
	switch(keyCode){
		case EKey::LEFT_CONTROL:
		case EKey::RIGHT_CONTROL:
			this->ctrlPressed = isDown;
			break;
		default:
			break;
	}
	return false;
}

void TextInput::OnCharacterInput(ting::Buffer<const std::uint32_t> unicode, EKey key){
	switch(key){
		case EKey::ENTER:
			break;
		case EKey::RIGHT:
			{
				size_t newIndex;
				if(this->ctrlPressed){
					bool spaceSkipped = false;
					newIndex = this->cursorIndex;
					for(auto i = this->Text().begin() + this->cursorIndex; i != this->Text().end(); ++i, ++newIndex){
						if(*i == std::uint32_t(' ')){
							if(spaceSkipped){
								break;
							}
						}else{
							spaceSkipped = true;
						}
					}

				}else{
					newIndex = this->cursorIndex + 1;
				}
				this->SetCursorIndex(newIndex);
			}
			break;
		case EKey::LEFT:
			if(this->cursorIndex == 0){
				break;
			}
			{
				size_t newIndex;
				if(this->ctrlPressed){
					bool spaceSkipped = false;
					newIndex = this->cursorIndex;
					for(auto i = this->Text().rbegin() + (this->Text().size() - this->cursorIndex);
							i != this->Text().rend();
							++i, --newIndex
						)
					{
						if(*i == std::uint32_t(' ')){
							if(spaceSkipped){
								break;
							}
						}else{
							spaceSkipped = true;
						}
					}
				}else{
					newIndex = this->cursorIndex - 1;
				}
				this->SetCursorIndex(newIndex);
			}
			break;
		case EKey::END:
			this->SetCursorIndex(this->Text().size());
			break;
		case EKey::HOME:
			this->SetCursorIndex(0);
			break;
		case EKey::BACKSPACE:
			if(this->cursorIndex != 0){
				auto t = this->Clear();
				t.erase(t.begin() + (this->cursorIndex - 1));
				this->SetText(std::move(t));
				this->SetCursorIndex(this->cursorIndex - 1);
			}
			break;
		case EKey::DELETE:
			if(this->cursorIndex < this->Text().size()){
				auto t = this->Clear();
				t.erase(t.begin() + this->cursorIndex);
				this->SetText(std::move(t));
			}
			break;
		default:
			if(unicode.size() != 0){
				auto t = this->Clear();
				t.insert(t.begin() + this->cursorIndex, unicode.begin(), unicode.end());
				this->SetText(std::move(t));
				
				this->SetCursorIndex(this->cursorIndex + unicode.size());
			}
			
			break;
		
	}
	
}
