#include "DataExtraction.h"

int read_csv(std::string file_name){

  io::CSVReader<4> in(file_name);

  in.read_header(io::ignore_extra_column, "day", "year", "month", "measurement");

  int day; int year; int month; double measurement;

  while(in.read_row(day, year, month, measurement)){

    date::year_month_day date{date::year(year), date::month(month), date::day(day)};

      if (!date.ok()) {
          std::cerr << "Invalid date: " << year << "-" << month << "-" << day << std::endl;
          continue; // Skip to the next row
      }

    std::cout << format("%F", date) << "," << measurement << std::endl;

  }

  return 0;

}

std::vector<Measurement> read_measurements(const std::string& filename) {
  // This code is WRONG! Can you spot the problems? It isn't easy, which is why
  // one of the reasons you don't want to write this kind of code yourself.
  std::ifstream file{filename};
  std::vector<Measurement> measurements;
  // Declaring a bunch of variables in advance, not good practice! Use a library
  // for reading CSV!
  int year, month, day, hour, minute, second, id;
  double signal, background, value;
  std::string header;
  // Assume first line is a header, read it first before we get the actual data
  std::getline(file, header);

  char comma;

  while (file >> year >> comma >> month >> comma >> day >> comma >> hour >>
         comma >> minute >> comma >> second >> comma >> id >> comma >> signal >>
         comma >> background >> comma >> value) {
    Measurement measurement{signal, background, id};
    if (measurement.is_valid()) {
      measurements.push_back(measurement);
    } else {
      std::cerr << "Warning: Skipping invalid measurement from (signal, "
                   "background, id): ("
                << signal << ", " << background << ", " << id << ")"
                << std::endl;
    }
  }
  return measurements;
}

void persist_measurements(const std::vector<Measurement>& measurements,
                          const std::string& output_filename) {
  // Note: We need .c_str() because TFile expects a C-string (char*) but
  // input_filename is a std::string.
  TFile output_file{output_filename.c_str(), "RECREATE"};
  TTree tree{"MNXB11", "MNXB11"};
  Double_t background;
  Double_t signal;
  Int_t id;
  tree.Branch("background", &background);
  tree.Branch("signal", &signal);
  tree.Branch("id", &id);

  for (const auto measurement : measurements) {
    id = measurement.get_id();
    background = measurement.get_background();
    signal = measurement.get_signal();
    tree.Fill();
  }
  tree.Write();
}