# Pumpensteuerung-Zisterne

## Systembeschreibung

Das hier beschriebene Projekt ist eine Pumpensteuerung für eine Zisterne oder einen Regenwassersammler. Ich besitze eine 1m tiefe Betonzisterne mit einem Fassungsvermögen von 1m³, die mit Regenwasser vom Dach gespeist wird. Hier muss ich sicherstellen, dass diese nicht überläuft. Andererseits möchte ich gezielt Wasser abpumpen, um dieses im Garten zu verrieseln oder Fässer zur allgemeinen Wasserentnahme zu füllen. Daraus ist dieses Projekt entstanden.
Kernstück der Anlage ist ein Steuergerät für eine Brunnenpumpe. Die Steuerung erfolgt mit einem Arduino Nano, dessen Firmware unter VSCode/PlatformIO mittels Arduino-Platform entwickelt wurde und hier abgelegt ist. Die Anzeigefunktion erfolgt über ein HD44780/1602 LCD-Display. Angeschlossen sind ein Konduktivsensor (Leitfähigkeitssensor), der über fünf Sonden in verschiedenen Wassertiefen, den Wasserstand im Behälter erfasst. Die Pegelstände werden angezeigt. Ein jahreszeitlich abhängiger Maximalpegel kann ausgewählt werden, der automatisch durch die Pumpensteuerung konstant gehalten wird. Über zwei Taster EIN/AUS, kann die Pumpe manuell gesteuert werden, um  Wassers gezielt zu entnehmen.
Ein weiterer Sensor überwacht die Temperatur und schaltet bei Frostgefahr die Anlage ab.
Eine Anpassung der Anlage, insbesondere des Konduktivsensors, auf eigene Belange, ist jederzeit möglich.

Eine detaillierte Beschreibung, Fotos und Bauunterlagen sind im Ordner [Dokumentation](Dokumentation) zu finden.
Das Projekt ist auch auf meiner Website [eltguy.de](https://eltguy.de) beschrieben.

## Lizenzierung

Die Firmware wird unter MIT-Lizenz veröffentlicht.

Steuergerät, Konduktivsensor und der Temperatursensors sind meine  Eigenentwicklungen und damit mein geistiges Eigentum. Diese Teile dürfen ausschließlich für private Anwendungen gebaut und verwendet werden. Ich gestatte auch die Nutzung der bereitgestellten Entwicklungsunterlagen, die ich zum Download anbiete. Eine kommerzielle Nutzung schließe ich definitiv aus!
Es ist nicht gestattet, Kennzeichnungen, die auf den geistigen Ursprung der Projekte hinweisen (Copyrights, Logos, Namen,…) aus Entwicklungsunterlagen der Anlage und Anlagenteilen zu entfernen.
Nachbau und Nutzung erfolgen auf eigenes Risiko. Ich hafte nicht für Schäden, die durch die Nutzung der Software, der Anlage oder Anlagenkomponenten entstehen.
