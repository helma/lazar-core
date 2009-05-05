require 'lazar'

#basename = "/home/ch/lazar-gui/public/data/cpdb/hamster_female_carcinogenicity/data/hamster_female_carcinogenicity"
basename = "/home/ch/lazar-gui/public/data/epafhm/lc50_mmol/data/lc50_mmol"
output = Lazar::getStringOut
p = Lazar::RegressionPredictor.new("#{basename}.smi", "#{basename}.act", "#{basename}.linfrag", "data/elements.txt", output)

p.predict_smi("CC=NN(C)C=OC")
y = p.get_yaml
puts y
#Process.detach fork { p.predict_smi("CC=NN(C)C=OC") }
#Process.detach fork { p.predict_smi("c1ccccc1N") }
