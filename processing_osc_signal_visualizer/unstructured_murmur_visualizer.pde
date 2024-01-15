import oscP5.*;
import netP5.*;

OscP5 oscP5Hex;
OscP5 oscP5Wiz;
LineBlob[] lineBlobsHex;
LineBlob[] lineBlobsWiz;

void setup() {
  size(800, 600);
  noSmooth(); // Disable antialiasing

  oscP5Hex = new OscP5(this, 3333);  // Receiving msgs from ESP32 HEX - port 3333
  oscP5Wiz = new OscP5(this, 4444);  // Receiving msgs from ESP32 WIZ - port 4444

  color[] rainbowColorsHex = {
    color(255, 69, 0),
    color(255, 140, 0),
    color(255, 215, 0),
    color(50, 205, 0),
    color(64, 224, 208),
    color(148, 0, 211)
  };

  color[] rainbowColorsWiz = {
    color(0, 191, 255),
    color(255, 20, 147),
    color(50, 205, 50),
    color(255, 165, 0),
    color(233, 30, 99),
    color(255, 255, 0)
  };

  lineBlobsHex = new LineBlob[rainbowColorsHex.length];
  for (int i = 0; i < lineBlobsHex.length; i++) {
    float yPos = i * 45 + 45;  // 45-pixel spacing between lines
    lineBlobsHex[i] = new LineBlob(yPos, rainbowColorsHex[i]);
  }

  lineBlobsWiz = new LineBlob[rainbowColorsWiz.length];
  for (int i = 0; i < lineBlobsWiz.length; i++) {
    float yPos = i * 45 + 45 + height / 2;
    lineBlobsWiz[i] = new LineBlob(yPos, rainbowColorsWiz[i]);
  }
}

void draw() {
  background(40, 40, 40);

  fill(120, 120, 120);
  textSize(16);

  // Unstructured Murmur title
  textAlign(RIGHT, TOP);
  text("Unstructured Murmur", width - 10, 10);

  // Names for lines HEX and WIZ
  fill(120, 120, 120);
  textSize(12);
  textAlign(RIGHT, CENTER);
  for (int i = 0; i < lineBlobsHex.length; i++) {
    text("HEX " + (i + 1), width - 10, lineBlobsHex[i].y);
  }

  for (int i = 0; i < lineBlobsWiz.length; i++) {
    text("WIZ " + (i + 1), width - 10, lineBlobsWiz[i].y);
  }

  for (LineBlob blob : lineBlobsHex) {
    blob.display();
  }
  for (LineBlob blob : lineBlobsWiz) {
    blob.display();
  }
}

void oscEvent(OscMessage theOscMessage) {
  String addr = theOscMessage.addrPattern();

  if (addr.startsWith("/hex/")) {
    int lineIndex = int(addr.substring(5)) - 1;
    if (lineIndex >= 0 && lineIndex < 6) {
      float value = theOscMessage.get(0).floatValue();
      boolean isPressed = value < 45;
      lineBlobsHex[lineIndex].update(isPressed);
    }
  } else if (addr.startsWith("/wiz/")) {
    int lineIndex = int(addr.substring(5)) - 1;
    if (lineIndex >= 0 && lineIndex < 6) {
      float value = theOscMessage.get(0).floatValue();
      boolean isPressed = value < 45;
      lineBlobsWiz[lineIndex].update(isPressed);
    }
  }
}
