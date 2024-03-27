void setup() {
    Serial.begin(9600);
}

void loop() {
    String phData = Serial.readStringUntil('$'); // Read until '$'
    
    // Extract pH and light values
    int lightIndex = phData.indexOf("L:");
    if (lightIndex != -1) { // Check if "L:" is found
        // Extract pH value
        int phIndex = phData.indexOf("PH:");
        float phValue = phData.substring(phIndex + 3, lightIndex).toFloat();

        // Extract light value
        int commaIndex = phData.indexOf(',', lightIndex);
        int lightValue = phData.substring(lightIndex + 3, commaIndex).toInt();

        // Print pH and light values
        Serial.print("PH Value: ");
        Serial.println(phValue, 2); // Print pH value with 2 decimal places
        Serial.print("Light Value: ");
        Serial.println(lightValue);
    }
     delay(1000); // Delay for 1 second
}
