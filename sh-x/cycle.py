import os
from typing import Union
import uuid

import qrcode
import qrcode.constants
import qrcode.image.svg


CODES_DIR = "qr-codes"

def delete_codes():
    import glob
    for file_path in glob.glob(f"{CODES_DIR}/*.png"):
        os.remove(file_path)
    
def generate_new_codes():
    for i in range(1, 21):
        qrcode.make(
            data=uuid.uuid4(),
            error_correction=qrcode.constants.ERROR_CORRECT_H,
            image_factory=qrcode.image.svg.SvgImage,
        ).save(f"{CODES_DIR}/{i:02d}.svg")

def cycle_and_export(page_num: Union[int, None] = None):
    # Cycle codes
    delete_codes()
    generate_new_codes()
    
    # Export to pdf
    if page_num is not None:
        os.system(f"soffice --headless --convert-to pdf --outdir output/{page_num} template.fodt > /dev/null")
        os.rename(f"output/{page_num}/template.pdf", f"output/page_{page_num}.pdf")
    else:
        os.system("soffice --headless --convert-to pdf --outdir output template.fodt > /dev/null")
        os.rename("output/template.pdf", f"output/page.pdf")
 
def do_again():
    while True:
        print("Would you like another page of QR codes? (y/n) ", end="")
        response = input().strip().lower()
        if response == "y":
            return True
        elif response == "n":
            return False
        else:
            print("Invalid response. Please enter 'y' or 'n'.\n")
    
 
if __name__ == "__main__":
    print("This script will cycle QR codes and export a page of codes to output/template.pdf")
    print("WARNING: Be sure NOT to print the same page twice! Badge IDs MUST be UNIQUE.")
    print()
    
    num_pages = 0
    while num_pages == 0 or  do_again():
        num_pages += 1
        
        print(f"Generating page {num_pages}... ", end="", flush=True) 
        cycle_and_export(num_pages)
        print("DONE")
        print("Page exported to output/page_{num_pages}.pdf")
    
    print(f"Generated a total of {num_pages} pages of QR codes.")
    