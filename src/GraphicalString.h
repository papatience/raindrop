#ifndef GGString_H_
#define GGString_H_

class GraphicalString : public GraphObject2D
{
	GString mText;
	Font* mFont;
public:
	GraphicalString();
	void SetText(GString _Text);
	void SetColour(float red, float green, float blue);
	GString GetText() const;
	void SetFont(Font* _Font);
	Font* GetFont() const;

	void Render();
};

#endif
