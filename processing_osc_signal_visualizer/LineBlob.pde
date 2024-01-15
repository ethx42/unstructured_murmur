class LineBlob {
  float y; // Vertical position of the line
  color lineColor; // Line color
  ArrayList<Float> pressures; // Pressure history

  LineBlob(float y, color c) {
    this.y = y;
    this.lineColor = c;
    this.pressures = new ArrayList<Float>();
  }

  // Update blob state based on OSC events
  void update(boolean isPressed) {
    int dataToAdd = 7; // Increase this number to speed up
    for (int i = 0; i < dataToAdd; i++) {
      if (isPressed) {
        pressures.add(0, 10.0);
      } else {
        pressures.add(0, 2.0);
      }
    }

    // Keep the list within the maximum size
    while (pressures.size() > width) {
      pressures.remove(pressures.size() - 1);
    }
  }

  void display() {
    int endX = width - 30; // Stop drawing 30px from the right edge

    for (int i = 0; i < pressures.size() && i < endX; i++) {
      float pressure = pressures.get(i);
      float barHeight = map(pressure, 2, 10, 0, 40); // Height of bars

      // Calculate opacity based on position for fading effect
      float fadeAmount = map(i, endX - 30, endX, 0, 1);
      fadeAmount = constrain(fadeAmount, 0, 1);
      color fadeColor = lerpColor(lineColor, color(0, 0, 0, 0), fadeAmount); // Use transparent color for fading

      stroke(fadeColor);
      strokeWeight(1);
      line(i, y - barHeight / 2, i, y + barHeight / 2);
    }
  }
}
