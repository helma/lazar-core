require 'lazar'

basename = "/home/ch/lazar-gui/public/data/cpdb/hamster_female_carcinogenicity/data/hamster_female_carcinogenicity"
#basename = "/home/ch/lazar-gui/public/data/cpdb/multi_cell_call/data/multi_cell_call"
output = Lazar::getStringStreamOut
p = Lazar::ClassificationPredictor.new("#{basename}.smi", "#{basename}.class", "#{basename}.linfrag", "data/elements.txt", output)

p.predict_smi("CC=NN(C)C=OC")
puts "finished"
y = p.get_yaml
puts y.class
puts y
#Process.detach fork { p.predict_smi("CC=NN(C)C=OC") }
#Process.detach fork { p.predict_smi("c1ccccc1N") }
