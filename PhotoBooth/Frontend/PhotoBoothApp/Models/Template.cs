using System.Collections.Generic;

namespace PhotoBooth.Models
{
    public class Template
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string BackgroundImagePath { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
        public List<PhotoZone> PhotoZones { get; set; } = new List<PhotoZone>();
        public List<TextElement> TextElements { get; set; } = new List<TextElement>();
        public List<GraphicElement> GraphicElements { get; set; } = new List<GraphicElement>();
        public string ThumbnailPath { get; set; }
    }

    public class PhotoZone
    {
        public int X { get; set; }
        public int Y { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
        public int Index { get; set; }
        public bool HasBorder { get; set; }
        public string BorderColor { get; set; }
        public int BorderWidth { get; set; }
        public int Rotation { get; set; }
    }

    public class TextElement
    {
        public string Text { get; set; }
        public int X { get; set; }
        public int Y { get; set; }
        public string FontFamily { get; set; }
        public int FontSize { get; set; }
        public string Color { get; set; }
        public bool Bold { get; set; }
        public bool Italic { get; set; }
        public int Rotation { get; set; }
    }

    public class GraphicElement
    {
        public string ImagePath { get; set; }
        public int X { get; set; }
        public int Y { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
        public int Rotation { get; set; }
        public double Opacity { get; set; } = 1.0;
    }
}
