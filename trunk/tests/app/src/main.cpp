#include "../../../src/morda/util/Quaternion.hpp"

#include "../../../src/morda/App.hpp"
#include "../../../src/morda/widgets/Widget.hpp"
#include "../../../src/morda/widgets/Container.hpp"

#include "../../../src/morda/widgets/Button.hpp"
#include "../../../src/morda/widgets/Label.hpp"

#include "../../../src/morda/shaders/SimpleSingleColoringShader.hpp"
#include "../../../src/morda/shaders/SimpleTexturingShader.hpp"

#include "../../../src/morda/resources/ResTexture.hpp"
#include "../../../src/morda/resources/ResFont.hpp"

#include "../../../src/morda/util/CharInputFocusable.hpp"

#include <ting/debug.hpp>
#include <ting/fs/FSFile.hpp>
#include <ting/util.hpp>

#include "../../../src/morda/util/ZipFile.hpp"



class SimpleWidget : public morda::Widget, public morda::Updateable, public morda::CharInputFocusable{
	ting::Ref<morda::ResTexture> tex;
	ting::Ref<morda::ResFont> fnt;
	
	SimpleWidget(const stob::Node& description) :
			morda::Widget(description)
	{
//		TRACE(<< "loading texture" << std::endl)
		this->tex = morda::App::Inst().ResMan().Load<morda::ResTexture>("tex_sample");
		this->fnt = morda::App::Inst().ResMan().Load<morda::ResFont>("fnt_main");
	}
public:	
	static inline ting::Ref<SimpleWidget> New(const stob::Node& description){
		return ting::Ref<SimpleWidget>(
				new SimpleWidget(description)
			);
	}
	
	ting::Inited<ting::u32, 0> timer;
	ting::Inited<ting::u32, 0> cnt;
	
	void Update(ting::u32 dt) OVERRIDE{
		this->timer += dt;
		++this->cnt;
		
		if(this->timer > 1000){
			this->timer -= 1000;
			
			TRACE(<< "Update(): UPS = " << this->cnt << std::endl)
			
			this->cnt = 0;
		}
	}
	
	bool OnMouseButton(bool isDown, const morda::Vec2f& pos, EMouseButton button, unsigned pointerId) OVERRIDE{
		TRACE(<< "OnMouseButton(): isDown = " << isDown << ", pos = " << pos << ", button = " << button << ", pointerId = " << pointerId << std::endl)
		if(!isDown){
			return false;
		}
		
		if(this->IsUpdating()){
			this->StopUpdating();
		}else{
			this->StartUpdating(30);
		}
		this->Focus();
		this->FocusCharInput();
		return true;
	}
	
	bool OnKey(bool isDown, morda::key::Key keyCode) OVERRIDE{
		if(isDown){
			TRACE(<< "SimpleWidget::OnKey(): down, keyCode = " << unsigned(keyCode) << std::endl)
			switch(keyCode){
				case morda::key::LEFT:
					TRACE(<< "SimpleWidget::OnKeyDown(): LEFT key caught" << std::endl)
					return true;
				case morda::key::A:
					TRACE(<< "SimpleWidget::OnKeyUp(): A key caught" << std::endl)
					return true;
				default:
					break;
			}
		}else{
			TRACE(<< "SimpleWidget::OnKey(): up, keyCode = " << unsigned(keyCode) << std::endl)
			switch(keyCode){
				case morda::key::LEFT:
					TRACE(<< "SimpleWidget::OnKeyUp(): LEFT key caught" << std::endl)
					return true;
				case morda::key::A:
					TRACE(<< "SimpleWidget::OnKeyUp(): A key caught" << std::endl)
					return true;
				default:
					break;
			}
		}
		return false;
	}
	
	void OnCharacterInput(const ting::Buffer<const ting::u32>& unicode) OVERRIDE{
		if(unicode.Size() == 0){
			return;
		}
		
		TRACE(<< "SimpleWidget::OnCharacterInput(): unicode = " << unicode[0] << std::endl)
	}
	
	void Render(const morda::Matr4f& matrix)const OVERRIDE{
		{
			morda::Matr4f matr(matrix);
			matr.Scale(this->Rect().d);

			this->tex->Tex().Bind();

			morda::SimpleTexturingShader &s = morda::App::Inst().Shaders().simpleTexturing;
			s.Bind();
			s.EnablePositionPointer();
//			s.SetColor(morda::Vec3f(1, 0, 0));
			s.SetMatrix(matr);
			s.DrawQuad01();
		}
		
//		this->fnt->Fnt().RenderTex(s , matrix);
		
//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glEnable(GL_BLEND);
//		morda::SimpleTexturingShader &s = morda::App::Inst().Shaders().simpleTexturing;
//		morda::Matr4f m(matrix);
//		m.Translate(200, 200);
//		this->fnt->Fnt().RenderString(s, m, "Hello World!");
	}
};



class CubeWidget : public morda::Widget, public morda::Updateable{
	ting::Ref<morda::ResTexture> tex;
	
	CubeWidget() : Widget(0){
		this->tex = morda::App::Inst().ResMan().Load<morda::ResTexture>("tex_sample");
		this->rot.Identity();
		this->StartUpdating(30);
	}
	
	morda::Quatf rot;
public:
	
	static ting::Ref<CubeWidget> New(){
		return ting::Ref<CubeWidget>(
				new CubeWidget()
			);
	}
	
	void Update(ting::u32 dt) OVERRIDE{
		this->rot %= morda::Quatf().InitRot(morda::Vec3f(1, 2, 1).Normalize(), 1.5f * (float(dt) / 1000));
	}
	
	void Render(const morda::Matr4f& matrix)const OVERRIDE{
		this->Widget::Render(matrix);
		
		morda::Matr4f matr(matrix);
		matr.Scale(this->Rect().d / 2);
		matr.Translate(1, 1);
		matr.Frustum(-2, 2, -1.5, 1.5, 2, 100);
		
		morda::Matr4f m(matr);
		m.Translate(0, 0, -4);
		
		m.Rotate(this->rot);

		this->tex->Tex().Bind();
		
		morda::SimpleTexturingShader &s = morda::App::Inst().Shaders().simpleTexturing;
		s.Bind();
		s.EnablePositionPointer();
		s.EnableTexCoordPointer();
//		s.SetColor(morda::Vec3f(0, 1, 0));
		s.SetMatrix(m);
		
		
		static morda::Vec3f cubePos[] = {
			morda::Vec3f(-1, -1, 1), morda::Vec3f(1, -1, 1), morda::Vec3f(-1, 1, 1),
			morda::Vec3f(1, -1, 1), morda::Vec3f(1, 1, 1), morda::Vec3f(-1, 1, 1),
			
			morda::Vec3f(1, -1, 1), morda::Vec3f(1, -1, -1), morda::Vec3f(1, 1, 1),
			morda::Vec3f(1, -1, -1), morda::Vec3f(1, 1, -1), morda::Vec3f(1, 1, 1),
			
			morda::Vec3f(1, -1, -1), morda::Vec3f(-1, -1, -1), morda::Vec3f(1, 1, -1),
			morda::Vec3f(-1, -1, -1), morda::Vec3f(-1, 1, -1), morda::Vec3f(1, 1, -1),
			
			morda::Vec3f(-1, -1, -1), morda::Vec3f(-1, -1, 1), morda::Vec3f(-1, 1, -1),
			morda::Vec3f(-1, -1, 1), morda::Vec3f(-1, 1, 1), morda::Vec3f(-1, 1, -1),
			
			morda::Vec3f(-1, 1, -1), morda::Vec3f(-1, 1, 1), morda::Vec3f(1, 1, -1),
			morda::Vec3f(-1, 1, 1), morda::Vec3f(1, 1, 1), morda::Vec3f(1, 1, -1),
			
			morda::Vec3f(-1, -1, -1), morda::Vec3f(1, -1, -1), morda::Vec3f(-1, -1, 1),
			morda::Vec3f(-1, -1, 1), morda::Vec3f(1, -1, -1), morda::Vec3f(1, -1, 1)
			
		};
		s.SetPositionPointer(cubePos);
		
		static morda::Vec2f cubeTex[] = {
			morda::Vec2f(0, 0), morda::Vec2f(1, 0), morda::Vec2f(0, 1),
			morda::Vec2f(1, 0), morda::Vec2f(1, 1), morda::Vec2f(0, 1),
			
			morda::Vec2f(0, 0), morda::Vec2f(1, 0), morda::Vec2f(0, 1),
			morda::Vec2f(1, 0), morda::Vec2f(1, 1), morda::Vec2f(0, 1),
			
			morda::Vec2f(0, 0), morda::Vec2f(1, 0), morda::Vec2f(0, 1),
			morda::Vec2f(1, 0), morda::Vec2f(1, 1), morda::Vec2f(0, 1),
			
			morda::Vec2f(0, 0), morda::Vec2f(1, 0), morda::Vec2f(0, 1),
			morda::Vec2f(1, 0), morda::Vec2f(1, 1), morda::Vec2f(0, 1),
			
			morda::Vec2f(0, 0), morda::Vec2f(1, 0), morda::Vec2f(0, 1),
			morda::Vec2f(1, 0), morda::Vec2f(1, 1), morda::Vec2f(0, 1),
			
			morda::Vec2f(0, 0), morda::Vec2f(1, 0), morda::Vec2f(0, 1),
			morda::Vec2f(1, 0), morda::Vec2f(1, 1), morda::Vec2f(0, 1)
		};
		s.SetTexCoordPointer(cubeTex);
		
		glEnable(GL_CULL_FACE);
		
		s.DrawArrays(GL_TRIANGLES, 36);
	}
};


class Application : public morda::App{
	inline static morda::App::WindowParams GetWindowParams()throw(){
		morda::App::WindowParams wp;
		
		wp.dim.x = 800;
		wp.dim.y = 800;
		
		return wp;
	}
public:
	Application() :
			App(GetWindowParams())
	{
//		this->ResMan().MountResPack(this->CreateResourceFileInterface());
		this->ResMan().MountResPack(morda::ZipFile::New(ting::fs::FSFile::New("res.zip")));
		
		this->inflater().AddWidget<SimpleWidget>("U_SimpleWidget");

		ting::Ref<morda::Widget> c = morda::App::Inst().inflater().Inflate(
				*this->CreateResourceFileInterface("test.gui.stob")
			);

//		morda::ZipFile zf(ting::fs::FSFile::New("res.zip"), "test.gui.stob");
//		ting::Ref<morda::Widget> c = morda::App::Inst().inflater().Inflate(zf);
		
		
		c->FindChildByName("show_VK_button").DynamicCast<morda::Button>()->pressed.Connect(static_cast<morda::App*>(this), &morda::App::ShowVirtualKeyboard);
		
		c->FindChildByName("Hello world button").DynamicCast<morda::Button>()->pressed.Connect(this, &Application::PostMessageToUIThread);
		
		
		if(ting::Ref<morda::Container> container = c.DynamicCast<morda::Container>()){
			ting::Ref<CubeWidget> w = CubeWidget::New();
			w->Resize(morda::Vec2f(200, 150));
			container->Add(w);
		}
		
		this->SetRootWidget(c);
	}
		
	class UIMessage : public ting::mt::Message{
		virtual void Handle() OVERRIDE{
			ASSERT_ALWAYS(morda::App::Inst().ThisIsUIThread())
			TRACE_ALWAYS(<< "UIMessage::Handle(): Print from UI thread!!!" << std::endl)
		}
	};
	
	void PostMessageToUIThread(){
		this->PostToUIThread_ts(ting::Ptr<ting::mt::Message>(new UIMessage()));
	}
};



ting::Ptr<morda::App> morda::CreateApp(int argc, const char** argv, const ting::Buffer<const ting::u8>& savedState){
	return ting::Ptr<Application>(new Application());
}
